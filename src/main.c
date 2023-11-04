#include <stdio.h>
#include <stdlib.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <curl/curl.h>

// Callback function to write downloaded data to a buffer
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    fwrite(contents, 1, total_size, (FILE *)userp);
    return total_size;
}

int main() {
    avformat_network_init(); // Initialize network components if needed

    char url[100];
    char output_file[100];

    printf("Enter the URL of the video: ");
    scanf("%s", url);

    printf("Enter the output file name: ");
    scanf("%s", output_file);

    // Initialize libcurl
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return 1;
    }

    // Open the output file
    FILE *output_fp = fopen(output_file, "wb");
    if (!output_fp) {
        fprintf(stderr, "Failed to open output file\n");
        curl_easy_cleanup(curl);
        return 1;
    }

    // Set libcurl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, output_fp);

    // Perform the download
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to download the video: %s\n", curl_easy_strerror(res));
        fclose(output_fp);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Close the libcurl handle
    curl_easy_cleanup(curl);

    // Close the output file
    fclose(output_fp);

    // FFmpeg code to process the downloaded video
    AVFormatContext* formatContext = NULL;
    int ret = avformat_open_input(&formatContext, output_file, NULL, NULL);

    printf("Enter the URL of the YouTube video: ");
    scanf("%s", url);
    printf("Enter the output file path: ");
    scanf("%s", output_file);

    if (ret < 0) {
        fprintf(stderr, "Error opening the input file: %s\n", av_err2str(ret));
        return 1;
    }

    ret = avformat_find_stream_info(formatContext, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error finding stream information: %s\n", av_err2str(ret));
        avformat_close_input(&formatContext);
        return 1;
    }

    av_dump_format(formatContext, 0, url, 0);

    const AVOutputFormat* outputFormat = av_guess_format(NULL, output_file, NULL);
    if (!outputFormat) {
        fprintf(stderr, "Error guessing output format.\n");
        avformat_close_input(&formatContext);
        return 1;
    }

    AVFormatContext* outputContext = NULL;
    ret = avformat_alloc_output_context2(&outputContext, outputFormat, NULL, output_file);
    if (ret < 0 || !outputContext) {
        fprintf(stderr, "Error creating output format context: %s\n", av_err2str(ret));
        avformat_close_input(&formatContext);
        return 1;
    }

    ret = avio_open(&outputContext->pb, output_file, AVIO_FLAG_WRITE);
    if (ret < 0) {
        fprintf(stderr, "Error opening output file: %s\n", av_err2str(ret));
        avformat_close_input(&formatContext);
        avformat_free_context(outputContext);
        return 1;
    }

    ret = avformat_write_header(outputContext, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error writing header: %s\n", av_err2str(ret));
        avformat_close_input(&formatContext);
        avformat_free_context(outputContext);
        return 1;
    }

    AVPacket packet;
    while (av_read_frame(formatContext, &packet) >= 0) {
        ret = av_write_frame(outputContext, &packet);
        av_packet_unref(&packet);
        if (ret < 0) {
            fprintf(stderr, "Error writing frame: %s\n", av_err2str(ret));
            break;
        }
    }

    ret = av_write_trailer(outputContext);
    if (ret < 0) {
        fprintf(stderr, "Error writing trailer: %s\n", av_err2str(ret));
    }

    avformat_close_input(&formatContext);
    avio_close(outputContext->pb);
    avformat_free_context(outputContext);

    return 0;
}