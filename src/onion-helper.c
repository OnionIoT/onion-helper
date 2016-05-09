#include <onion-helper.h>

#include <libubox/uloop.h>
#include <libubus.h>


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


//// initialize onion-helper with the ubus
// onion-helper 
int onion_helper_init(struct ubus_context *ctx) {
	static const struct ubus_method onionHelperMethods[] = {
		UBUS_METHOD ("background", 	onionHelperBackgroundMethod, 	ohBackgroundPolicy),
		UBUS_METHOD	("echo", 		onionHelperEchoMethod, 			ohEchoPolicy),
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

	return 0;
}