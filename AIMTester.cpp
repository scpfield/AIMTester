/*
AIMTester.cpp : This file contains the C code to test the AIM AWS server.
*/

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>
#include <iostream>


/* Function declarations */

DWORD WINAPI thread_proc(LPVOID param);

BOOL GET_Request(
    LPCWSTR server, 
    LPCWSTR path, 
    HANDLE h_heap, 
    LPSTR* p_buf_start,
    DWORD* dw_total_size);

BOOL POST_Request(
    LPCWSTR server,
    LPCWSTR path,
    HANDLE h_heap,
    LPSTR* p_resp_buf_start,
    DWORD* dw_resp_total_size,
    LPCSTR p_post_data);


/* Per-thread data struct */
typedef struct _THREAD_DATA
{
    int thread_num;
    LPCWSTR target_server;

} THREAD_DATA;



/* Client thread implementation */
DWORD WINAPI thread_proc(LPVOID param)
{
    THREAD_DATA* p_thread_data = (THREAD_DATA*)param;
    //LPCWSTR path = L"/dev/skus";
    LPCWSTR path = L"/api/v1/create";
    LPCSTR p_post_data = NULL;
    HANDLE h_heap = NULL;
    LPSTR p_buf = NULL;
    DWORD dw_total_size = 0;
    BOOL b_exit_flag = FALSE;

    if ((h_heap = HeapCreate(
        0,
        0,
        0)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    //GET_Request(p_thread_data->target_server, path, h_heap, &p_buf, &dw_total_size);
    //printf("Thread %d: GET response size = %d\n", p_thread_data->thread_num, dw_total_size);
    //printf("DATA\n");
    //printf(p_buf);

    //p_post_data = "{\"sku\": \"09351234\", \"description\": \"foobar123\", \"price\": \"2.99\"}";
    p_post_data = "{\"name\":\"05634s\",\"salary\":\"5653\",\"age\":\"34\"}";

    printf("post data = %s", p_post_data);

    POST_Request(p_thread_data->target_server, path, h_heap, &p_buf, &dw_total_size, p_post_data);
    printf("Thread %d: POST response size = %d\n", p_thread_data->thread_num, dw_total_size);
    printf("DATA\n");
    printf(p_buf);


    //GET_Request(p_thread_data->target_server, path, h_heap, &p_buf, &dw_total_size);
    //printf("Thread %d: GET response size = %d\n", p_thread_data->thread_num, dw_total_size);
    //printf("DATA\n");
    //printf(p_buf);

    b_exit_flag = TRUE;
    goto cleanup;

cleanup:

    if (p_buf) HeapFree(h_heap, 0, p_buf);
    if (h_heap) HeapDestroy(h_heap);
     
    return b_exit_flag;

}


/*  Performs a GET request to a server/path.
    Fills out a buffer with the content. */

BOOL GET_Request(
    LPCWSTR server, 
    LPCWSTR path, 
    HANDLE h_heap, 
    LPSTR* p_buf_start,
    DWORD* dw_total_size)
{
    HINTERNET h_session = NULL;
    HINTERNET h_conn = NULL;
    HINTERNET h_req = NULL;
    LPSTR p_buf_current = *p_buf_start;
    DWORD dw_bytes_avail = 0;
    DWORD dw_bytes_recv = 0;
    BOOL b_result = FALSE;
    BOOL b_exit_flag = FALSE;

    if ((h_session = WinHttpOpen(
        L"scpfield_agent/1.0",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 
        0)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    if ((h_conn = WinHttpConnect(
        h_session, 
        server,
        INTERNET_DEFAULT_HTTPS_PORT, 
        0)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    if ((h_req = WinHttpOpenRequest(
        h_conn, 
        L"GET", 
        path,
        NULL, 
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    if ((b_result = WinHttpSendRequest(
        h_req,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0, 
        0, 
        0)) == FALSE)
    {
        b_exit_flag = FALSE;
        goto cleanup;

    }

    if ((b_result = WinHttpReceiveResponse(
        h_req, 
        NULL)) == FALSE)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    *dw_total_size = 0;

    if ((*p_buf_start = (LPSTR) HeapAlloc(
        h_heap, 
        HEAP_ZERO_MEMORY, 
        *dw_total_size)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    while (TRUE)
    {
        if ((b_result = WinHttpQueryDataAvailable(
            h_req,
            &dw_bytes_avail)) == FALSE)
        {
            b_exit_flag = FALSE;
            goto cleanup;
        }

        if (dw_bytes_avail < 1) break;

        *dw_total_size += dw_bytes_avail;

        if ((*p_buf_start = (LPSTR) HeapReAlloc(
            h_heap,
            HEAP_ZERO_MEMORY,
            *p_buf_start,
            *dw_total_size + 1)) == NULL)
        {
            b_exit_flag = FALSE;
            goto cleanup;
        }

        p_buf_current = *p_buf_start + (*dw_total_size - dw_bytes_avail);

        if ((b_result = WinHttpReadData(
            h_req,
            p_buf_current,
            dw_bytes_avail,
            &dw_bytes_recv)) == FALSE)
        {
            b_exit_flag = FALSE;
            goto cleanup;
        }

    }

    b_exit_flag = TRUE;
    goto cleanup;


cleanup:

    if (b_exit_flag == FALSE) wprintf(L"GetLastError() = %u\n", GetLastError());
    if (h_req) WinHttpCloseHandle(h_req);
    if (h_conn) WinHttpCloseHandle(h_conn);
    if (h_session) WinHttpCloseHandle(h_session);
    return b_exit_flag;

}


BOOL POST_Request(
    LPCWSTR server,
    LPCWSTR path,
    HANDLE h_heap,
    LPSTR* p_resp_buf_start,
    DWORD* dw_resp_total_size,
    LPCSTR p_post_data)
{
    HINTERNET h_session = NULL;
    HINTERNET h_conn = NULL;
    HINTERNET h_req = NULL;
    LPSTR p_buf_current = *p_resp_buf_start;
    DWORD dw_bytes_avail = 0;
    DWORD dw_bytes_recv = 0;
    BOOL b_result = FALSE;
    BOOL b_exit_flag = FALSE;

    if ((h_session = WinHttpOpen(
        L"scpfield_agent/1.0",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    if ((h_conn = WinHttpConnect(
        h_session,
        server,
        INTERNET_DEFAULT_HTTPS_PORT,
        0)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }


    if ((h_req = WinHttpOpenRequest(
        h_conn,
        L"POST",
        path,
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    if ((b_result = WinHttpSendRequest(
        h_req,
        L"Content-Type: application/json",
        -1,
        &p_post_data,
        strlen(p_post_data),
        strlen(p_post_data),
        NULL)) == FALSE)
    {
        printf("GetLastError = %u\n", GetLastError());
        b_exit_flag = FALSE;
        goto cleanup;

    }

    if ((b_result = WinHttpReceiveResponse(
        h_req,
        NULL)) == FALSE)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    *dw_resp_total_size = 0;

    if ((*p_resp_buf_start = (LPSTR) HeapAlloc(
        h_heap,
        HEAP_ZERO_MEMORY,
        *dw_resp_total_size)) == NULL)
    {
        b_exit_flag = FALSE;
        goto cleanup;
    }

    while (TRUE)
    {
        if ((b_result = WinHttpQueryDataAvailable(
            h_req,
            &dw_bytes_avail)) == FALSE)
        {
            b_exit_flag = FALSE;
            goto cleanup;
        }

        if (dw_bytes_avail < 1) break;

        *dw_resp_total_size += dw_bytes_avail;

        if ((*p_resp_buf_start = (LPSTR)HeapReAlloc(
            h_heap,
            HEAP_ZERO_MEMORY,
            *p_resp_buf_start,
            *dw_resp_total_size + 1)) == NULL)
        {
            b_exit_flag = FALSE;
            goto cleanup;
        }

        p_buf_current = *p_resp_buf_start + (*dw_resp_total_size - dw_bytes_avail);

        if ((b_result = WinHttpReadData(
            h_req,
            p_buf_current,
            dw_bytes_avail,
            &dw_bytes_recv)) == FALSE)
        {
            b_exit_flag = FALSE;
            goto cleanup;
        }

    }

    b_exit_flag = TRUE;
    goto cleanup;


cleanup:

    if (b_exit_flag == FALSE) wprintf(L"GetLastError() = %u\n", GetLastError());
    if (h_req) WinHttpCloseHandle(h_req);
    if (h_conn) WinHttpCloseHandle(h_conn);
    if (h_session) WinHttpCloseHandle(h_session);
    return b_exit_flag;




}


/* main function, launches threads, waits
for them to stop */

int main()
{
    const DWORD max_threads = 1;
    HANDLE h_threads[max_threads] = { 0 };
    THREAD_DATA r_thread_data[max_threads] = { 0 };
    int i = 0;

    for (i = 0; i < max_threads; i++)
    {
        r_thread_data[i].thread_num = i;
        //r_thread_data[i].target_server = L"1ryu4whyek.execute-api.us-west-2.amazonaws.com";
        r_thread_data[i].target_server = L"www.dummy.restapiexample.com";

        h_threads[i] = CreateThread(NULL, 0, thread_proc, &r_thread_data[i], 0, NULL);

    }

    for (i = 0; i < max_threads; i++)
    {
        if (h_threads[i] != NULL)
        {
            WaitForSingleObject(h_threads[i], INFINITE);
            CloseHandle(h_threads[i]);
        }
    }


 }
