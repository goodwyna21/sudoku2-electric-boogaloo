#ifndef GETBOARD_C
#define GETBOARD_C

#include <stdio.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

const char* BOARD_API_LINK = "https://sudoku-game-and-api.netlify.app/api/sudoku";

typedef struct {
    unsigned char *buffer;
    size_t len;
    size_t buflen;
} get_request;

#define CHUNK_SIZE 2048

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb; 
    get_request *req = (get_request *) userdata;

    printf("receive chunk of %zu bytes\n", realsize);

    while (req->buflen < req->len + realsize + 1)
    {
        req->buffer = realloc(req->buffer, req->buflen + CHUNK_SIZE);
        req->buflen += CHUNK_SIZE;
    }
    memcpy(&req->buffer[req->len], ptr, realsize);
    req->len += realsize;
    req->buffer[req->len] = 0;

    return realsize;
}


//gets board from api
bool getBoard(int* board, int difficulty){
    printf("Fetching board...\n");
    CURL *curl;
    CURLcode res;

    get_request req = {.buffer = NULL, .len = 0, .buflen = 0};
    const int boardDataSize = 179;
    char boardData[boardDataSize];
    const int dataStarts[] = {317, 508, 697};

    curl_global_init(CURL_GLOBAL_DEFAULT);    
    curl = curl_easy_init();
    if(curl){
        curl_easy_setopt(curl, CURLOPT_URL, BOARD_API_LINK);
        curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);

        req.buffer = malloc(CHUNK_SIZE);
        req.buflen = CHUNK_SIZE;

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

//        printf("Result = %u\n",res);
//        printf("Total received bytes: %zu\n", req.len);
        //printf("Received data:\n%s\n", req.buffer);
//UNSAFE I AM STUPID
        strncpy(boardData, (char*)req.buffer+dataStarts[difficulty]-1, boardDataSize);
        //printf("boardData: %s\n", boardData);

        int index = 0;
        for(int i = 0; i < boardDataSize; i++){
            if(48 > boardData[i] || boardData[i] > 57) continue;
            board[index] = boardData[i] - 48;
            index++;
        }
    }
    
    free(req.buffer); 
    curl_easy_cleanup(curl);
    curl_global_cleanup();
        
    return curl;
}

//{"game":"Sudoku","created_by":"Amit Sharma","info":"Each array in the data array represents a row in the sudoku grid.","data":[[6,3,8,4,1,7,2,9,5],[2,1,7,3,5,9,8,6,4],[5,9,4,2,8,6,7,3,1],[8,2,3,1,9,5,4,7,6],[7,4,9,6,3,2,5,1,8],[1,6,5,8,7,4,3,2,9],[4,5,6,9,2,3,1,8,7],[3,7,1,5,6,8,9,4,2],[9,8,2,7,4,1,6,5,3]],"easy":[
//[0,0,0,4,1,0,0,9,5],[2,0,7,3,0,9,8,6,0],[5,9,0,2,0,0,7,3,1],[8,2,3,1,9,5,4,7,6],[0,4,0,6,3,2,5,1,0],[0,0,5,8,7,4,3,0,0],[4,5,6,9,2,3,0,0,7],[3,7,0,5,6,0,9,0,0],[0,8,2,7,4,1,6,5,3]

//{"game":"Sudoku","created_by":"Amit Sharma","info":"Each array in the data array represents a row in the sudoku grid.","data":[[1,2,7,6,9,8,4,5,3],[5,9,8,1,3,4,7,2,6],[6,3,4,5,2,7,8,9,1],[8,5,2,3,4,1,9,6,7],[3,1,6,9,7,2,5,4,8],[7,4,9,8,5,6,1,3,2],[2,8,1,4,6,9,3,7,5],[9,7,3,2,1,5,6,8,4],[4,6,5,7,8,3,2,1,9]],"easy":[[1,2,7,0,0,0,0,5,3],[5,9,8,0,3,0,7,2,0],[6,3,4,5,2,7,8,9,1],[0,5,2,0,4,1,9,6,0],[3,1,0,9,0,0,0,0,8],[7,4,9,8,5,0,1,3,0],[2,0,1,4,0,9,0,0,5],[9,7,3,2,1,5,6,8,4],[4,6,0,0,0,3,2,1,0]],"medium":[
//{"game":"Sudoku","created_by":"Amit Sharma","info":"Each array in the data array represents a row in the sudoku grid.","data":[[1,2,7,6,9,8,4,5,3],[5,9,8,1,3,4,7,2,6],[6,3,4,5,2,7,8,9,1],[8,5,2,3,4,1,9,6,7],[3,1,6,9,7,2,5,4,8],[7,4,9,8,5,6,1,3,2],[2,8,1,4,6,9,3,7,5],[9,7,3,2,1,5,6,8,4],[4,6,5,7,8,3,2,1,9]],"easy":[[1,2,7,0,0,0,0,5,3],[5,9,8,0,3,0,7,2,0],[6,3,4,5,2,7,8,9,1],[0,5,2,0,4,1,9,6,0],[3,1,0,9,0,0,0,0,8],[7,4,9,8,5,0,1,3,0],[2,0,1,4,0,9,0,0,5],[9,7,3,2,1,5,6,8,4],[4,6,0,0,0,3,2,1,0]],"medium":[[0,0,0,0,9,0,4,0,3],[0,9,8,1,0,0,7,2,0],[0,0,0,5,0,7,0,0,1],[0,5,2,3,0,1,9,0,7],[3,1,6,0,0,0,5,4,8],[0,0,0,8,0,0,0,0,0],[2,8,0,0,6,9,0,0,0],[9,7,0,0,0,5,6,0,4],[4,0,5,0,0,0,0,1,0]],"hard":[
#endif