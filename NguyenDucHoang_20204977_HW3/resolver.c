#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <regex.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define FILE_NAME "html.txt"
#define LINK_FILE "links.csv"
#define TEXT_FILE "texts.csv"

int is_IP_address(char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if ((str[i] < '0' || '9' < str[i]) && str[i] != '.')
        {
            return 0;
        }
    }
    return 1;
}

void getHTMLFile(const char *url)
{
    CURL *curlHandle = curl_easy_init();

    curl_easy_setopt(curlHandle, CURLOPT_URL, url);
    FILE *file = fopen(FILE_NAME, "w+");

    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, file);
    curl_easy_perform(curlHandle);
    fclose(file);
    curl_easy_cleanup(curlHandle);
}

char *getHTMLString()
{
    char *html = NULL;
    size_t html_size = 0;
    FILE *file = fopen(FILE_NAME, "r");

    if (file == NULL)
    {
        perror("Error opening HTML file");
        return NULL;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file))
    {
        size_t len = strlen(buffer);
        char *new_html = realloc(html, html_size + len + 1);
        if (new_html == NULL)
        {
            perror("Memory allocation error");
            free(html);
            fclose(file);
            return NULL;
        }
        html = new_html;
        strcpy(html + html_size, buffer);
        html_size += len;
    }
    fclose(file);
    return html;
}

char *getFullUrl(const char *url)
{
    char *fullUrl = malloc(strlen(url) + strlen("https://") + 1);
    strcpy(fullUrl, "https://");
    strcat(fullUrl, url);
    return fullUrl;
}

void extract_hyperlinks(char *html, char ***links, char ***text, int *num_texts, int *num_links)
{
    regex_t hl_regex_links;
    regex_t hl_regex_text;
    regcomp(&hl_regex_links, "<a\\s+href=\"([^\"]+)\"[^>]*>", REG_ICASE | REG_EXTENDED);
    regcomp(&hl_regex_text, "<h3[^>]*><a[^>]*>([^<]*)</a></h3>", REG_ICASE | REG_EXTENDED);

    regmatch_t pmatch[2];
    *num_links = 0;
    char **link_list = NULL;
    const char *cursor = html;

    while (regexec(&hl_regex_links, cursor, 2, pmatch, 0) == 0)
    {
        int start = pmatch[1].rm_so;
        int end = pmatch[1].rm_eo;

        char *link = malloc(end - start + 1);
        strncpy(link, cursor + start, end - start);
        link[end - start] = '\0';

        char **new_link_list = realloc(link_list, (*num_links + 1) * sizeof(char *));
        if (new_link_list == NULL)
        {
            perror("Memory allocation error");
            for (int i = 0; i < *num_links; i++)
            {
                free(link_list[i]);
            }
            free(link_list);
            free(html);
            regfree(&hl_regex_links);
            return;
        }
        link_list = new_link_list;
        link_list[(*num_links)++] = link;

        cursor += end;
    }

    cursor = html;
    char **text_list = NULL;
    *num_texts = 0;
    while (regexec(&hl_regex_text, cursor, 2, pmatch, 0) == 0)
    {
        int start = pmatch[1].rm_so;
        int end = pmatch[1].rm_eo;

        char *text = malloc(end - start + 1);
        strncpy(text, cursor + start, end - start);
        text[end - start] = '\0';

        char **new_text_list = realloc(text_list, (*num_texts + 1) * sizeof(char *));
        if (new_text_list == NULL)
        {
            perror("Memory allocation error");
            for (int i = 0; i < *num_texts; i++)
            {
                free(text_list[i]);
            }
            free(text_list);
            free(html);
            regfree(&hl_regex_text);
            return;
        }
        text_list = new_text_list;
        text_list[(*num_texts)++] = text;

        cursor += end;
    }
    *text = text_list;

    *links = link_list;

    free(html);
    regfree(&hl_regex_links);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <url>\n", argv[0]);
        return 1;
    }
    char *temp = argv[1];
    struct hostent *he;
    struct in_addr **addr_list;
    struct in_addr addr;
    if (is_IP_address(temp))
    {
        // nếu là địa chỉ IP
        // biến đổi địa chỉ IP từ dạng "số và dấu chấm" thành dạng nhị phân
        inet_aton(temp, &addr);
        // tìm kiếm thông tin của địa chỉ IP
        he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
        // nếu không tìm thấy
        if (he == NULL)
        {
            printf("Not found information\n");
            exit(1);
        }
        // ngược lại, in thông tin
        printf("Official name: %s\n", he->h_name);
        if (he->h_aliases[0] != NULL)
        {
            printf("Alias name:\n");
            for (int i = 0; he->h_aliases[i] != NULL; i++)
            {
                printf("%s\n", he->h_aliases[i]);
            }
        }
    }
    else
    {
        // nếu là tên miền
        // tìm kiếm thông tin của tiên miền
        he = gethostbyname(temp);
        // nếu không tìm thấy
        if (he == NULL)
        {
            printf("Not found information\n");
            exit(1);
        }
        // ngược lại, in thông tin
        addr_list = (struct in_addr **)he->h_addr_list;
        printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
        if (addr_list[1] != NULL)
        {
            printf("Alias IP:\n");
            for (int i = 1; addr_list[i] != NULL; i++)
            {
                printf("%s\n", inet_ntoa(*addr_list[i]));
            }
        }
    }
    char *fullUrl = getFullUrl(temp);
    getHTMLFile(fullUrl);
    char **final_links = NULL;
    int final_count = 0;
    char **final_texts = NULL;
    int final_count_text = 0;
    extract_hyperlinks(getHTMLString(), &final_links, &final_texts, &final_count_text, &final_count);
    // write to file
    FILE *file = fopen(LINK_FILE, "w+");
    for (int i = 0; i < final_count; i++)
    {
        fprintf(file, "%s\n", final_links[i]);
    }
    fclose(file);
    file = fopen(TEXT_FILE, "w+");
    for (int i = 0; i < final_count_text; i++)
    {
        fprintf(file, "%s\n", final_texts[i]);
    }
    fclose(file);
    return 0;
}