#include <onion-helper.h>

#include <libubox/uloop.h>
#include <libubus.h>

#include <fcntl.h>
#include <unistd.h>


static struct blob_buf b;

////// UBUS //////
//// policies
enum {
	OH_BACKGROUND_CMD,
	OH_BACKGROUND_PARAMS,
	__OH_BACKGROUND_MAX,
};

static const struct blobmsg_policy ohBackgroundPolicy[__OH_BACKGROUND_MAX] = {
	[OH_BACKGROUND_CMD]		= { .name = "command", .type = BLOBMSG_TYPE_STRING },
	[OH_BACKGROUND_PARAMS]	= { .name = "params",  .type = BLOBMSG_TYPE_ARRAY  },
};

enum {
	OH_ECHO_WORD,
	__OH_ECHO_MAX,
};

static const struct blobmsg_policy ohEchoPolicy[__OH_ECHO_MAX] = {
	[OH_ECHO_WORD]		= { .name = "message", .type = BLOBMSG_TYPE_STRING },
};

enum {
	OH_WRITE_PATH,
	OH_WRITE_DATA,
	OH_WRITE_APPEND,
	OH_WRITE_MODE,
	OH_WRITE_BASE64,
	__OH_WRITE_MAX,
};

static const struct blobmsg_policy ohWritePolicy[__OH_WRITE_MAX] = {
	[OH_WRITE_PATH]   = { .name = "path",   .type = BLOBMSG_TYPE_STRING },
	[OH_WRITE_DATA]   = { .name = "data",   .type = BLOBMSG_TYPE_STRING },
	[OH_WRITE_APPEND] = { .name = "append", .type = BLOBMSG_TYPE_BOOL   },
	[OH_WRITE_MODE]   = { .name = "mode",   .type = BLOBMSG_TYPE_INT32  },
	[OH_WRITE_BASE64] = { .name = "base64", .type = BLOBMSG_TYPE_BOOL   },
};

enum {
	OH_DOWNLOAD_URL,
	OH_DOWNLOAD_PATH,
	OH_DOWNLOAD_BACKGROUND,
	__OH_DOWNLOAD_MAX,
};

static const struct blobmsg_policy ohDownloadPolicy[__OH_DOWNLOAD_MAX] = {
	[OH_DOWNLOAD_URL]			= { .name = "url",  .type = BLOBMSG_TYPE_STRING  },
	[OH_DOWNLOAD_PATH]			= { .name = "path", .type = BLOBMSG_TYPE_STRING },
	[OH_DOWNLOAD_BACKGROUND]	= { .name = "background", .type = BLOBMSG_TYPE_BOOL   },
};



static int
ubus_errno_status(void)
{
	switch (errno)
	{
	case EACCES:
		return UBUS_STATUS_PERMISSION_DENIED;

	case ENOTDIR:
		return UBUS_STATUS_INVALID_ARGUMENT;

	case ENOENT:
		return UBUS_STATUS_NOT_FOUND;

	case EINVAL:
		return UBUS_STATUS_INVALID_ARGUMENT;

	default:
		return UBUS_STATUS_UNKNOWN_ERROR;
	}
}

//// functions that implement methods for the onion-helper ubus group
// background function
static int
onionHelperBackgroundMethod(	struct ubus_context *ctx, struct ubus_object *obj,
								struct ubus_request_data *req, const char *method,
								struct blob_attr *msg)
{	
	struct 	blob_attr *tb[__OH_BACKGROUND_MAX];
	struct 	blob_attr *cur;
	
	int 	rem;
	int 	numArgs;
	int 	pid;
	int 	count;

	char 	**arguments;


	// parse the json input
	blobmsg_parse(	ohBackgroundPolicy, __OH_BACKGROUND_MAX, tb,
					blob_data(msg), blob_len(msg));

	if (!tb[OH_BACKGROUND_CMD])
		return UBUS_STATUS_INVALID_ARGUMENT;

	// set the arguments
	OH_DBG_PRINT("Allocating arguments\n");
	numArgs 		= 2;
	arguments 		= malloc(sizeof(char *) * numArgs);
	if (!arguments) {
		return UBUS_STATUS_UNKNOWN_ERROR;
	}

	OH_DBG_PRINT("Setting inital arguments\n");
	arguments[0] 	= blobmsg_data(tb[OH_BACKGROUND_CMD]);	// set the command
	arguments[1]	= NULL;

	OH_DBG_PRINT("Looping through arguments\n");
	if (tb[OH_BACKGROUND_PARAMS]) {
		blobmsg_for_each_attr(cur, tb[OH_BACKGROUND_PARAMS], rem)
		{
			if (blobmsg_type(cur) != BLOBMSG_TYPE_STRING) {
				continue;
			}

			numArgs++;

			// reallocate the arguments array to hold an additional element
			OH_DBG_PRINT("Reallocating arguments\n");
			arguments 	= realloc(arguments, (sizeof(char*) * numArgs) );
			if (!arguments) {
				return UBUS_STATUS_UNKNOWN_ERROR;
			}	

			arguments[numArgs - 2] 	= blobmsg_data(cur);	// set new argument
			arguments[numArgs - 1] 	= NULL;					// set last element to NULL
		}
	}

	// DEBUG:
	for (count = 0; count < numArgs-1; count++) {
		OH_DBG_PRINT("argument: %s\n", arguments[count]);
	}

	// launch the process in the background
	pid 	= backgroundLaunch(arguments);
	OH_DBG_PRINT("backgroundLaunch is done!\n");

	// response
	blob_buf_init(&b, 0);

	blobmsg_add_string	(&b, "success", "ok");
	blobmsg_add_u32		(&b, "pid",  pid);
	ubus_send_reply	(ctx, req, b.head);


	// clean-up
	blob_buf_free(&b);

	return UBUS_STATUS_OK;
}

// echo function
static int
onionHelperEchoMethod		(	struct ubus_context *ctx, struct ubus_object *obj,
								struct ubus_request_data *req, const char *method,
								struct blob_attr *msg)
{	
	struct 	blob_attr *tb[__OH_ECHO_MAX];
	struct 	blob_attr *cur;
	
	int 	rem;
	int 	numArgs;
	int 	count;

	char 	*word;

	// parse the json input
	blobmsg_parse(	ohEchoPolicy, __OH_ECHO_MAX, tb,
					blob_data(msg), blob_len(msg));

	if (!tb[OH_ECHO_WORD])
		return UBUS_STATUS_INVALID_ARGUMENT;

	// read the string to echo back
	word 	= malloc(sizeof(char *));
	strcpy(word, blobmsg_data(tb[OH_ECHO_WORD]) );


	// response 
	blob_buf_init(&b, 0);

	blobmsg_add_string(&b, "echo", word);
	ubus_send_reply(ctx, req, b.head);


	// clean-up
	free(word);
	blob_buf_free(&b);

	return UBUS_STATUS_OK;
}

// write function
static int
onionHelperWriteMethod		(	struct ubus_context *ctx, struct ubus_object *obj,
								struct ubus_request_data *req, const char *method,
								struct blob_attr *msg)
{	
	struct 	blob_attr *tb[__OH_WRITE_MAX];
	int 		append = O_TRUNC;
	int 		fd, rv = 0;
	int 		status = EXIT_SUCCESS;

	mode_t 		prev_mode, mode = 0666;
	
	void 		*data = NULL;
	ssize_t 	data_len = 0;


	// parse the json input
	blobmsg_parse(	ohWritePolicy, __OH_WRITE_MAX, tb,
					blob_data(msg), blob_len(msg));

	// check for the path and data arguments
	if (!tb[OH_WRITE_PATH] || !tb[OH_WRITE_DATA])
		return UBUS_STATUS_INVALID_ARGUMENT;

	data = blobmsg_data(tb[OH_WRITE_DATA]);
	data_len = blobmsg_data_len(tb[OH_WRITE_DATA]) - 1;

	// read the append and mode arguments
	if (tb[OH_WRITE_APPEND] && blobmsg_get_bool(tb[OH_WRITE_APPEND]))
		append = O_APPEND;

	if (tb[OH_WRITE_MODE])
		mode = blobmsg_get_u32(tb[OH_WRITE_MODE]);

	// open the file
	prev_mode = umask(0);
	fd = open(blobmsg_data(tb[OH_WRITE_PATH]), O_CREAT | O_WRONLY | append, mode);
	umask(prev_mode);

	if (fd < 0) {
		rv 		= ubus_errno_status();
		status 	= EXIT_FAILURE;
	}

	// perform the base64 actions
	if (tb[OH_WRITE_BASE64] && blobmsg_get_bool(tb[OH_WRITE_BASE64]))
	{
		data_len = b64_decode(data, data, data_len);
		if (data_len < 0)
		{
			rv = UBUS_STATUS_UNKNOWN_ERROR;
		}
	}

	if (status == EXIT_SUCCESS) {
		if (write(fd, data, data_len) < 0) {
			rv = -1;
		}
	}

	if (fsync(fd) < 0) {
		rv 	= -1;
	}

	close(fd);
	sync();

	if (rv) {
		status = ubus_errno_status();
	}

	// send back the response
	blob_buf_init(&b, 0);

	blobmsg_add_u32		(&b, "code",  status);
	blobmsg_add_u32		(&b, "bytes", (int)data_len);
	ubus_send_reply	(ctx, req, b.head);
	// clean-up
	blob_buf_free(&b);

	
	return status;
}

void _downloadSendResponse(struct ubus_context *ctx, struct ubus_request_data *req, int status, bool background, int errorNum)
{
	// response 
	blob_buf_init(&b, 0);

	blobmsg_add_u32(&b, "status", status);
	blobmsg_add_u8(&b, "background", (background == true) ? 0x01 : 0x00);
	if (status != EXIT_SUCCESS) {
		blobmsg_add_string(&b, "error", strerror(errorNum) );
	}
	
	ubus_send_reply(ctx, req, b.head);

	// clean-up
	blob_buf_free(&b);
}

// download function
static int
onionHelperDownloadMethod	(	struct ubus_context *ctx, struct ubus_object *obj,
								struct ubus_request_data *req, const char *method,
								struct blob_attr *msg)
{	
	struct 	blob_attr *tb[__OH_DOWNLOAD_MAX];
	struct 	blob_attr *cur;
	pid_t	pid;
	int 	status, errorNum;

	// parse the json input
	blobmsg_parse(	ohDownloadPolicy, __OH_DOWNLOAD_MAX, tb,
					blob_data(msg), blob_len(msg));

	if (!tb[OH_DOWNLOAD_PATH] || !tb[OH_DOWNLOAD_URL]) {
		return UBUS_STATUS_INVALID_ARGUMENT;
	}

	if (tb[OH_DOWNLOAD_BACKGROUND] && blobmsg_get_bool(tb[OH_DOWNLOAD_BACKGROUND]) )
	{
		// perform the download in the background
		pid 	= fork();

		if (pid == 0) {
			// child process
			status = setpgid(0, 0);
			status = downloadFile(blobmsg_data(tb[OH_DOWNLOAD_URL]), blobmsg_data(tb[OH_DOWNLOAD_PATH]), &errorNum );
			exit(0);
		}
		else {
			// send ubus response
			_downloadSendResponse(ctx, req, EXIT_SUCCESS, true, errorNum);
		}
	}
	else {
		// perform the download
		status = downloadFile(blobmsg_data(tb[OH_DOWNLOAD_URL]), blobmsg_data(tb[OH_DOWNLOAD_PATH]), &errorNum );
		// send ubus response
		_downloadSendResponse(ctx, req, status, false, errorNum);
	}

	return UBUS_STATUS_OK;
}


//// initialize onion-helper with the ubus
// onion-helper 
int onion_helper_init(struct ubus_context *ctx) {
	static const struct ubus_method onionHelperMethods[] = {
		UBUS_METHOD ("background", 	onionHelperBackgroundMethod, 	ohBackgroundPolicy),
		UBUS_METHOD	("echo", 		onionHelperEchoMethod, 			ohEchoPolicy),
		UBUS_METHOD	("write", 		onionHelperWriteMethod, 		ohWritePolicy),
		UBUS_METHOD	("download", 	onionHelperDownloadMethod, 		ohDownloadPolicy),
	};

	static struct ubus_object_type onionHelperObject_type =
		UBUS_OBJECT_TYPE("onion-helper", onionHelperMethods);

	static struct ubus_object onionHelperObject = {
		.name = "onion-helper",
		.type = &onionHelperObject_type,
		.methods = onionHelperMethods,
		.n_methods = ARRAY_SIZE(onionHelperMethods),
	};

	return ubus_add_object(ctx, &onionHelperObject);
}
///////////////


int main(int argc, char** argv)
{
	static struct ubus_context 	*ctx;
	const char 					*ubus_socket = NULL;

	curlInit();
	uloop_init();

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	onion_helper_init(ctx);

	uloop_run();
	ubus_free(ctx);
	uloop_done();

	curlCleanup();

	return 0;
}