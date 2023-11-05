#include <curl/curl.h>
#include <tidy/tidy.h>
#include <tidy/buffio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char **response_ptr = (char **)userp;
    *response_ptr = (char *)realloc(*response_ptr, realsize + 1);
    if (*response_ptr) {
        memcpy(*response_ptr, contents, realsize);
        (*response_ptr)[realsize] = 0;
    }
    return 0;
}

int main() {
    CURL *curl;
    CURLcode res;

    // initate lib curl
    curl = curl_easy_init();
    if (curl) {
        char *url;
        char *response;

        printf("Enter URL: ");
        scanf("%s", url);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res!= CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            printf("Download completed successfully");
        }
        
        fclose(fp);
        free(url);
        free(outputFile);
        curl_easy_cleanup(curl);
    }
    return 0;
}