//#include <node.h>

#include "tagnode.hpp"

using namespace std;
using namespace v8;

struct Baton {
	// libuv's request struct.
	uv_work_t request;

	// This handle holds the callback function we'll call after the work request
	// has been completed in a threadpool thread. It's persistent so that V8
	// doesn't garbage collect it away while our request waits to be processed.
	// This means that we'll have to dispose of it later ourselves.
	v8::Persistent<v8::Function> callback;

	// Tracking errors that happened in the worker function. You can use any
	// variables you want. E.g. in some cases, it might be useful to report
	// an error number.
	bool error;
	std::string error_message;

	// Custom data you can pass through.
	TagNode *obj;
};

// This function is executed in another thread at some point after it has been
// scheduled. IT MUST NOT USE ANY V8 FUNCTIONALITY. Otherwise your extension
// will crash randomly and you'll have a lot of fun debugging.
// If you want to use parameters passed into the original call, you have to
// convert them to PODs or some other fancy method.
void AsyncWork(uv_work_t* req) {
	Baton* baton = static_cast<Baton*>(req->data);

	// Do work in threadpool here.
	TagLib::FileRef f(baton->obj->_path); //Hay que hacer delete f?

	baton->obj->_tag = (!f.isNull() && f.tag());
	if(baton->obj->_tag) {
		TagLib::Tag *tag = f.tag(); //Hay que hacer delete tag?

		//TODO: These vars must be freed in destructor
		baton->obj->_title = (char*) malloc(tag->title().size());
		strcpy(baton->obj->_title, tag->title().toCString());

		baton->obj->_artist = (char*) malloc(tag->artist().size());
		strcpy(baton->obj->_artist, tag->artist().toCString());

		baton->obj->_album = (char*) malloc(tag->album().size());
		strcpy(baton->obj->_album, tag->album().toCString());

		baton->obj->_year = tag->year();

		baton->obj->_comment = (char*) malloc(tag->comment().size());
		strcpy(baton->obj->_comment, tag->comment().toCString());

		baton->obj->_track = tag->track();

		baton->obj->_genre = (char*) malloc(tag->genre().size());
		strcpy(baton->obj->_genre, tag->genre().toCString());

	}

	baton->obj->audioProperties = !f.isNull() && f.audioProperties();
	if(baton->obj->audioProperties) {

		TagLib::AudioProperties *properties = f.audioProperties(); //Hay que hacer delete properties?

		//TODO: These vars must be freed in destructor
		baton->obj->_bitrate = properties->bitrate();
		baton->obj->_sample_rate = properties->sampleRate();
		baton->obj->_channels = properties->channels();
		baton->obj->_length = properties->length();
	}

	// If the work we do fails, set baton->error_message to the error string
	// and baton->error to true.

	// TODO: set baton->error to true if data wasnot found
}

// This function is executed in the main V8/JavaScript thread. That means it's
// safe to use V8 functions again. Don't forget the HandleScope!
void AsyncAfter(uv_work_t* req) {
	HandleScope scope;
	Baton* baton = static_cast<Baton*>(req->data);

	if (baton->error) { //TODO: Handle error
		Local<Value> err = Exception::Error(String::New(baton->error_message.c_str()));

		// Prepare the parameters for the callback function.
		const unsigned argc = 1;
		Local<Value> argv[argc] = { err };

		// Wrap the callback function call in a TryCatch so that we can call
		// node's FatalException afterwards. This makes it possible to catch
		// the exception from JavaScript land using the
		// process.on('uncaughtException') event.
		TryCatch try_catch;
		baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
		if (try_catch.HasCaught()) {
			node::FatalException(try_catch);
		}
	} else {
		// In case the operation succeeded, convention is to pass null as the
		// first argument before the result arguments.
		// In case you produced more complex data, this is the place to convert
		// your plain C++ data structures into JavaScript/V8 data structures.
		const unsigned argc = 2;
		//TODO: aprender como pasar de un struct a un objeto de js
		Local<Value> argv[argc] = {
			Local<Value>::New(Null()),
			Local<Value>::New(Integer::New(1))
		};

		// Wrap the callback function call in a TryCatch so that we can call
		// node's FatalException afterwards. This makes it possible to catch
		// the exception from JavaScript land using the
		// process.on('uncaughtException') event.
		TryCatch try_catch;
		baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
		if (try_catch.HasCaught()) {
			node::FatalException(try_catch);
		}
	}

	// The callback is a permanent handle, so we have to dispose of it manually.
	baton->callback.Dispose();
	delete baton;
}


Persistent<FunctionTemplate> TagNode::constructor;

void TagNode::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	Local<String> name = String::NewSymbol("TagNode");

	constructor = Persistent<FunctionTemplate>::New(tpl);
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	constructor->InstanceTemplate()->SetInternalFieldCount(1);
	constructor->SetClassName(name);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(constructor, "read", Read);
	NODE_SET_PROTOTYPE_METHOD(constructor, "path", Path);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "title", Title);
	//NODE_SET_PROTOTYPE_METHOD(constructor, "artist", Artist);
	NODE_SET_PROTOTYPE_METHOD(constructor, "album", Album);
	NODE_SET_PROTOTYPE_METHOD(constructor, "year", Year);
	NODE_SET_PROTOTYPE_METHOD(constructor, "comment", Comment);
	NODE_SET_PROTOTYPE_METHOD(constructor, "track", Track);
	NODE_SET_PROTOTYPE_METHOD(constructor, "genre", Genre);

	//Properties
	constructor->InstanceTemplate()->SetAccessor(String::New("title"), GetTitle, SetTitle);
	constructor->InstanceTemplate()->SetAccessor(String::New("artist"), GetArtist, SetArtist);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	target->Set(name, constructor->GetFunction());
}

//TagNode::TagNode(Local<String> path)
TagNode::TagNode(char *path)
	: ObjectWrap(),
	_path(path) {}


Handle<Value> TagNode::New(const Arguments& args) {
	HandleScope scope;

	if (!args.IsConstructCall()) {
		return ThrowException(Exception::TypeError(
			String::New("Use the new operator to create instances of this object."))
				);
	}

	if (args.Length() < 1) {
		return ThrowException(Exception::TypeError(
			String::New("First argument must be the path of the file")));
	}

	v8::String::AsciiValue v8Str(args[0]);
	//TODO: cStr must be free in the destructor
	char *cStr = (char*) malloc(strlen(*v8Str) + 1);
	strcpy(cStr, *v8Str);
	//TagNode* obj = new TagNode(args[0]->ToString());
	TagNode* obj = new TagNode(cStr);
	obj->Wrap(args.This());

	return args.This();
}



Handle<Value> TagNode::Read(const Arguments& args) {
	HandleScope scope;

	if (!args[0]->IsFunction()) {
		return ThrowException(Exception::TypeError(
			String::New("First argument must be a callback function")));
	}

	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());

	// There's no ToFunction(), use a Cast instead.
	Local<Function> callback = Local<Function>::Cast(args[0]);

	// This creates our work request, including the libuv struct.
	Baton* baton = new Baton();
	baton->request.data = baton;
	baton->callback = Persistent<Function>::New(callback);
	baton->obj = obj;

	// Schedule our work request with libuv. Here you can specify the functions
	// that should be executed in the threadpool and back in the main thread
	// after the threadpool function completed.
	int status = uv_queue_work(uv_default_loop(), &baton->request, AsyncWork, AsyncAfter);
	assert(status == 0);

	return Undefined();
}


/*Handle<Value> TagNode::Read(const Arguments& args) {
	HandleScope scope;

	// Retrieves the pointer to the wrapped object instance.
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());

	TagLib::FileRef f(obj->_path); //Hay que hacer delete f?

	if(!f.isNull() && f.tag()) {
		TagLib::Tag *tag = f.tag();

		//TODO: These vars must be freed in destructor
		obj->_title = (char*) malloc(tag->title().size());
		strcpy(obj->_title, tag->title().toCString());

		obj->_artist = (char*) malloc(tag->artist().size());
		strcpy(obj->_artist, tag->artist().toCString());

		obj->_album = (char*) malloc(tag->album().size());
		strcpy(obj->_album, tag->album().toCString());

		obj->_year = tag->year();

		obj->_comment = (char*) malloc(tag->comment().size());
		strcpy(obj->_comment, tag->comment().toCString());

		obj->_track = tag->track();

		obj->_genre = (char*) malloc(tag->genre().size());
		strcpy(obj->_genre, tag->genre().toCString());

	}

	if(!f.isNull() && f.audioProperties()) {

		TagLib::AudioProperties *properties = f.audioProperties();

		//TODO: These vars must be freed in destructor
		obj->_bitrate = properties->bitrate();
		obj->_sample_rate = properties->sampleRate();
		obj->_channels = properties->channels();
		obj->length = properties->length();
	}

	return scope.Close(Boolean::New(1));
}*/


Handle<Value> TagNode::Path(const Arguments& args) {
	HandleScope scope;
	// Retrieves the pointer to the wrapped object instance.
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	//return scope.Close(obj->_path);
	return scope.Close(String::New(obj->_path));
}
/*Handle<Value> TagNode::Title(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(String::New(obj->_title));
}*/

/* Setters & getters of js props*/
Handle<Value> TagNode::GetTitle(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	return scope.Close(String::New(obj->_title));
}
void TagNode::SetTitle(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;//TODO:
	//unwrapTag(info)->tag->setTitle(NodeStringToTagLibString(value));
}
Handle<Value> TagNode::GetArtist(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	return scope.Close(String::New(obj->_artist));
}
void TagNode::SetArtist(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;//TODO:
	//unwrapTag(info)->tag->setTitle(NodeStringToTagLibString(value));
}
//TODO: All setters & getters of js props


Handle<Value> TagNode::Album(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(String::New(obj->_album));
}
Handle<Value> TagNode::Year(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(Integer::New(obj->_year));
}
Handle<Value> TagNode::Comment(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(String::New(obj->_comment));
}
Handle<Value> TagNode::Track(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(Integer::New(obj->_track));
}
Handle<Value> TagNode::Genre(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(String::New(obj->_genre));
}





void RegisterModule(Handle<Object> target) {
	TagNode::Init(target);
}

NODE_MODULE(tagnode, RegisterModule);
