/* handler.c: HTTP Request Handlers */

#include "spidey.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/* Internal Declarations */
Status handle_browse_request(Request *request);
Status handle_file_request(Request *request);
Status handle_cgi_request(Request *request);
Status handle_error(Request *request, Status status);

/**
 * Handle HTTP Request.
 *
 * @param   r           HTTP Request structure
 * @return  Status of the HTTP request.
 *
 * This parses a request, determines the request path, determines the request
 * type, and then dispatches to the appropriate handler type.
 *
 * On error, handle_error should be used with an appropriate HTTP status code.
 **/
Status  handle_request(Request *r) {
    Status result;

    /* Parse request */
    result = parse_request(r);
    if(result == -1) {
        debug("error while parsing request");
        handle_error(r, HTTP_STATUS_BAD_REQUEST);
        return HTTP_STATUS_BAD_REQUEST;
    }
    debug("r->method: %s", r->method);

    /* Determine request path */
    r->path = determine_request_path(r->uri);
    if(r->path == NULL) {
        debug("path is NULL");
        handle_error(r, HTTP_STATUS_NOT_FOUND);
        return HTTP_STATUS_NOT_FOUND; 
    }

    debug("HTTP REQUEST PATH: %s", r->path);

    /* Dispatch to appropriate request handler type based on file type */
    log("HTTP REQUEST STATUS: %s", http_status_string(result));
    
    struct stat stats;

    // fill file stat struct 
    if(stat(r->path, &stats) != 0) {
        debug("Unable to get file properties");
        debug("file: %s may not exist", r->path);
        handle_error(r, HTTP_STATUS_BAD_REQUEST);
        return HTTP_STATUS_BAD_REQUEST;   
    }

    // path is directory
    if(S_ISDIR(stats.st_mode)) {
        handle_browse_request(r);    
    }
    else if (stats.st_mode & S_IXUSR){
        handle_cgi_request(r);    
    }
    else{
    	handle_file_request(r);
    }

    return result;
}

/**
 * Handle browse request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP browse request.
 *
 * This lists the contents of a directory in HTML.
 *
 * If the path cannot be opened or scanned as a directory, then handle error
 * with HTTP_STATUS_NOT_FOUND.
 **/

Status  handle_browse_request(Request *r) {
    struct dirent **entries;
    char buffer[BUFSIZ];
    size_t nread;
 
    debug("handling a BROWSE");

    /* Open a directory for reading or scanning */
    int n = scandir(r->path, &entries, 0, alphasort);
	if (n < 0){
	    debug("scandir failed: %s", strerror(errno));
	    handle_error(r, HTTP_STATUS_NOT_FOUND);
            return HTTP_STATUS_NOT_FOUND;
	}

    /* Write HTTP Header with OK Status and text/html Content-Type */
    fprintf(r->stream, "HTTP/1.0 200 OK\r\n");
    fprintf(r->stream, "Content-Type: text/html\r\n");
    fprintf(r->stream, "\r\n");

    fprintf(r->stream, "<h1><center>Path: %s</center></h1>", r->uri);
    if(streq(r->uri, "/")) {
        fprintf(r->stream, "<h2><font size = 25><strong>- Welcome to the Mainframe -</strong></font><h2>");
    }

    /* For each entry in directory, emit HTML list item */	
    char *directoryPath = realpath("directory.html", NULL);
    FILE *fs = fopen(directoryPath, "r");
    if (!fs){
        debug("error: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    // write fs to socket stream
    nread = fread(buffer, 1, BUFSIZ, fs);
    while (nread > 0){
    	fwrite(buffer, 1, nread, r->stream);
    	nread = fread(buffer, 1, BUFSIZ, fs);
    }    

    fprintf(r->stream, "<ul>\n");

    // loop through entries in the directory
    for (int i = 0; i < n; i++){
        // skip current directory
	if (streq(entries[i]->d_name, ".")){
	    free(entries[i]);
            continue;
	}
        //debug("IN BROWSE - r->uri: %s", r->uri);
        //debug("IN BROWSE - ENTRY NAME: %s", entries[i]->d_name);
        //debug("RPATH: %s", r->path);
	char last = r->uri[strlen(r->uri)-1];
        if (last == '/'){
    	    fprintf(r->stream, "<li style=\"font-size: 30px\"><a href=\"%s%s\">%s</a></li>\n", r->uri, entries[i]->d_name, entries[i]->d_name);
	}
	else{
            fprintf(r->stream, "<li style=\"font-size: 30px\"><a href=\"%s/%s\">%s</a></li>\n", r->uri, entries[i]->d_name, entries[i]->d_name);
	}

	free(entries[i]);
    }

    free(entries);

    // image html    
    fprintf(r->stream, "</ul>\n");
    fprintf(r->stream, "<center><img src=https://files.slack.com/files-pri/T0HJVP8MS-F012N5U6PM1/image.png>");
    fprintf(r->stream, "<img src=https://files.slack.com/files-pri/T0HJVP8MS-F01277K2GDD/image.png></center>");
	
    free(directoryPath);
    fclose(fs);

    /* Return OK */
    return HTTP_STATUS_OK;
}

/**
 * Handle file request.
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This opens and streams the contents of the specified file to the socket.
 *
 * If the path cannot be opened for reading, then handle error with
 * HTTP_STATUS_NOT_FOUND.
 **/

Status  handle_file_request(Request *r) {
    FILE *fs;
    char buffer[BUFSIZ];
    char *mimetype = NULL;
    size_t nread;

    debug("handling a FILE");

    /* Open file for reading */
    debug("OPENING r->path = %s", r->path);
    fs = fopen(r->path, "r");
    if (!fs){
        debug("error: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }
    log("r->path OPENED");

    /* Determine mimetype */
    mimetype = determine_mimetype(r->path);
    if(mimetype == NULL) {
        debug("invalid mimetype"); 
    }
    debug("mimetype: %s", mimetype);

    /* Write HTTP Headers with OK status and determined Content-Type */
    fprintf(r->stream, "HTTP/1.0 200 OK\r\n");
    fprintf(r->stream, "Content-Type: %s\r\n", mimetype);
    fprintf(r->stream, "\r\n");

    /* Read from file and write to socket in chunks */
    debug("READING from r->path: %s", r->path);
    debug("WRITING to r->stream");
    nread = fread(buffer, 1, BUFSIZ, fs);
    while (nread > 0){
    	fwrite(buffer, 1, nread, r->stream);
    	nread = fread(buffer, 1, BUFSIZ, fs);
    }

    /* Close file, deallocate mimetype, return OK */
    debug("closing file, freeing mimetype, return OK");
    fclose(fs);
    free(mimetype);
    
    return HTTP_STATUS_OK;

}

/**
 * Handle CGI request
 *
 * @param   e           HTTP Request structure.
 * @return  Status of the HTTP file request.
 *
 * This popens and streams the results of the specified executables to the
 * socket.
 *
 * If the path cannot be popened, then handle error with
 * HTTP_STATUS_INTERNAL_SERVER_ERROR.
 **/

Status  handle_cgi_request(Request *r) {
    FILE *pfs;
    char buffer[BUFSIZ];
    size_t nread;   

    debug("handling a CGI");

    /* Export CGI environment variables from request:
     * http://en.wikipedia.org/wiki/Common_Gateway_Interface */

    // use setenv()
    setenv("DOCUMENT_ROOT", RootPath, 1);
    setenv("QUERY_STRING", r->query, 1);
    setenv("REMOTE_ADDR", r->host, 1);
    setenv("REMOTE_PORT", r->port, 1);
    setenv("REQUEST_METHOD", r->method, 1);
    setenv("REQUEST_URI", r->uri, 1);
    setenv("SCRIPT_FILENAME", r->path, 1);
    setenv("SERVER_PORT", Port, 1);
    
    /* Export CGI environment variables from request headers */
    // setenv for headers
    for (Header *header = r->headers; header; header = header->next){
    	if (streq(header->name, "Accept"))
    	    setenv("HTTP_ACCEPT", header->data, 1);
    	else if (streq(header->name, "Host"))
    	    setenv("HTTP_HOST", header->data, 1);
    	else if (streq(header->name, "Accept-Language"))
    	    setenv("HTTP_ACCEPT_LANGUAGE", header->data, 1);
    	else if (streq(header->name, "Accept-Encoding"))
    	    setenv("HTTP_ACCEPT_ENCODING", header->data, 1);
    	else if (streq(header->name, "Connection"))
    	    setenv("HTTP_CONNECTION", header->data, 1);
    	else if (streq(header->name, "User-Agent"))
	    setenv("HTTP_USER_AGENT", header->data, 1);
    }

    /* POpen CGI Script */
    pfs = popen(r->path, "r");

    /* Copy data from popen to socket */
    debug("READING from PFS r->path: %s", r->path);
    debug("WRITING to r->stream");
    
    nread = fread(buffer, 1, BUFSIZ, pfs);
    while (nread > 0){
    	fwrite(buffer, 1, nread, r->stream);
    	nread = fread(buffer, 1, BUFSIZ, pfs);
    }

    /* Close popen, return OK */
    pclose(pfs);
    return HTTP_STATUS_OK;
}

/**
 * Handle displaying error page
 *
 * @param   r           HTTP Request structure.
 * @return  Status of the HTTP error request.
 *
 * This writes an HTTP status error code and then generates an HTML message to
 * notify the user of the error.
 **/

Status  handle_error(Request *r, Status status) {
    const char *status_string = http_status_string(status);
    char buffer[BUFSIZ];
    size_t nread;

    debug("handling an ERROR");

    /* Write HTTP Header */
    fprintf(r->stream, "HTTP/1.0 %s\r\n", status_string);
    fprintf(r->stream, "Content-Type: text/html\r\n");
    fprintf(r->stream, "\r\n");

    /* Write HTML Description of Error*/
    fprintf(r->stream, "<h1><center>%s</center></h1>\n", status_string);
    char *errorPath = realpath("error.html", NULL);
    FILE *fs = fopen(errorPath, "r");
    if (!fs){
        debug("error: %s", strerror(errno));
        return HTTP_STATUS_NOT_FOUND;
    }

    // write fs to socket stream    
    nread = fread(buffer, 1, BUFSIZ, fs);
    while (nread > 0){
    	fwrite(buffer, 1, nread, r->stream);
    	nread = fread(buffer, 1, BUFSIZ, fs);
    }

    fclose(fs);
    free(errorPath);
    
    /* Return specified status */
    return status;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
