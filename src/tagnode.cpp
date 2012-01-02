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

	baton->obj->_audioProperties = !f.isNull() && f.audioProperties();
	if(baton->obj->_audioProperties) {

		TagLib::AudioProperties *properties = f.audioProperties(); //Hay que hacer delete properties?

		//TODO: These vars must be freed in destructor
		baton->obj->_bitrate = properties->bitrate();
		baton->obj->_samplerate = properties->sampleRate();
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
		Local<Object> retobj = Object::New();
		if (baton->obj->_tag) {
			retobj->Set(String::New("tag"), Boolean::New(baton->obj->_tag));
			retobj->Set(String::New("title"), String::New(baton->obj->_title));
			retobj->Set(String::New("path"), String::New(baton->obj->_path));
			retobj->Set(String::New("artist"), String::New(baton->obj->_artist));
			retobj->Set(String::New("album"), String::New(baton->obj->_album));
			retobj->Set(String::New("year"), Integer::New(baton->obj->_year));
			retobj->Set(String::New("comment"), String::New(baton->obj->_comment));
			retobj->Set(String::New("track"), Integer::New(baton->obj->_track));
			retobj->Set(String::New("genre"), String::New(baton->obj->_genre));
		}
		if (baton->obj->_audioProperties) {
			retobj->Set(String::New("audioProperties"), Boolean::New(baton->obj->_audioProperties));
			retobj->Set(String::New("bitrate"), Number::New(baton->obj->_bitrate));
			retobj->Set(String::New("sample_rate"), Number::New(baton->obj->_samplerate));
			retobj->Set(String::New("channels"), Integer::New(baton->obj->_channels));
			retobj->Set(String::New("length"), Integer::New(baton->obj->_length));
		}
		// In case the operation succeeded, convention is to pass null as the
		// first argument before the result arguments.
		// In case you produced more complex data, this is the place to convert
		// your plain C++ data structures into JavaScript/V8 data structures.
		const unsigned argc = 2;
		//TODO: aprender como pasar de un struct a un objeto de js
		Local<Value> argv[argc] = {
			Local<Value>::New(Null()),
			retobj
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
	NODE_SET_PROTOTYPE_METHOD(constructor, "tag", Tag);
	NODE_SET_PROTOTYPE_METHOD(constructor, "audioProperties", AudioProperties);

	//Properties
	constructor->InstanceTemplate()->SetAccessor(String::New("title"), GetTitle, SetTitle);
	constructor->InstanceTemplate()->SetAccessor(String::New("artist"), GetArtist, SetArtist);
	constructor->InstanceTemplate()->SetAccessor(String::New("album"), GetAlbum, SetAlbum);
	constructor->InstanceTemplate()->SetAccessor(String::New("year"), GetYear, SetYear);
	constructor->InstanceTemplate()->SetAccessor(String::New("comment"), GetComment, SetArtist);
	constructor->InstanceTemplate()->SetAccessor(String::New("track"), GetTrack, SetTrack);
	constructor->InstanceTemplate()->SetAccessor(String::New("genre"), GetGenre, SetGenre);
	constructor->InstanceTemplate()->SetAccessor(String::New("bitrate"), GetBitrate, SetBitrate);
	constructor->InstanceTemplate()->SetAccessor(String::New("samplerate"), GetSamplerate, SetSamplerate);
	constructor->InstanceTemplate()->SetAccessor(String::New("channels"), GetChannels, SetChannels);
	constructor->InstanceTemplate()->SetAccessor(String::New("length"), GetLength, SetLength);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	target->Set(name, constructor->GetFunction());
}

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
	//TODO: cStr (TagNode->_path) must be free in the destructor
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

	//FIXME: should't we use scope.Close()?
	return Undefined();
}

Handle<Value> TagNode::Path(const Arguments& args) {
	HandleScope scope;
	// Retrieves the pointer to the wrapped object instance.
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	//return scope.Close(obj->_path);
	return scope.Close(String::New(obj->_path));
}

/* Setters & getters of js props*/
Handle<Value> TagNode::GetTitle(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(String::New(obj->_title));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetTitle(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	free(obj->_title);
	v8::String::AsciiValue v8Str(value);
	obj->_title = (char*) malloc(strlen(*v8Str) + 1);
	strcpy(obj->_title, *v8Str);
}

Handle<Value> TagNode::GetArtist(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(String::New(obj->_artist));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetArtist(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	free(obj->_artist);
	v8::String::AsciiValue v8Str(value);
	obj->_artist = (char*) malloc(strlen(*v8Str) + 1);
	strcpy(obj->_artist, *v8Str);
}

Handle<Value> TagNode::GetAlbum(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(String::New(obj->_album));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetAlbum(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	free(obj->_album);
	v8::String::AsciiValue v8Str(value);
	obj->_album = (char*) malloc(strlen(*v8Str) + 1);
	strcpy(obj->_album, *v8Str);
}

Handle<Value> TagNode::GetYear(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(Integer::New(obj->_year));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetYear(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	obj->_year = value->ToInteger()->Value();
}

Handle<Value> TagNode::GetComment(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(String::New(obj->_comment));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetComment(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	free(obj->_comment);
	v8::String::AsciiValue v8Str(value);
	obj->_comment = (char*) malloc(strlen(*v8Str) + 1);
	strcpy(obj->_comment, *v8Str);
}

Handle<Value> TagNode::GetTrack(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(Integer::New(obj->_track));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetTrack(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	obj->_track = value->ToInteger()->Value();
}

Handle<Value> TagNode::GetGenre(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(String::New(obj->_genre));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetGenre(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	free(obj->_genre);
	v8::String::AsciiValue v8Str(value);
	obj->_genre = (char*) malloc(strlen(*v8Str) + 1);
	strcpy(obj->_genre, *v8Str);
}

Handle<Value> TagNode::GetBitrate(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(Number::New(obj->_bitrate));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetBitrate(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	obj->_bitrate = value->ToNumber()->Value();
}

Handle<Value> TagNode::GetSamplerate(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(Number::New(obj->_samplerate));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetSamplerate(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	obj->_samplerate = value->ToNumber()->Value();
}

Handle<Value> TagNode::GetChannels(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(Integer::New(obj->_channels));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetChannels(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	obj->_channels = value->ToInteger()->Value();
}

Handle<Value> TagNode::GetLength(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	if (obj->_tag)
		return scope.Close(Integer::New(obj->_length));
	else //FIXME: should't we use scope.Close()?
		return Undefined();
}
void TagNode::SetLength(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(info.Holder());
	obj->_length = value->ToInteger()->Value();
}

Handle<Value> TagNode::Tag(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(Boolean::New(obj->_tag));
}
Handle<Value> TagNode::AudioProperties(const Arguments& args) {
	HandleScope scope;
	TagNode* obj = ObjectWrap::Unwrap<TagNode>(args.This());
	return scope.Close(Boolean::New(obj->_audioProperties));
}

//Register the module.
void RegisterModule(Handle<Object> target) {
	TagNode::Init(target);
}

NODE_MODULE(tagnode, RegisterModule);
