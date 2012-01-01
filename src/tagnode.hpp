#ifndef TAGNODE_HPP
#define TAGNODE_HPP

#include <iostream>
#include <stdio.h>

#include <node.h>

#include <string.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>


// Do not include this line. It's generally frowned upon to use namespaces
// in header files as it may cause issues with other code that includes your
// header file.
// using namespace v8;


// Forward declaration.
void AsyncWork(uv_work_t* req);
void AsyncAfter(uv_work_t* req);

// We use a struct to store information about the asynchronous "work request".
struct Baton;

class TagNode : public node::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);

	//TODO: All setters & getters of js props
	static v8::Handle<v8::Value> GetTitle(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetTitle(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetArtist(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetArtist(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	//Vars made public, lazynees of coding setters
	//TODO: Make destructor to free vars
	int _tag;
	char *_title;
	char *_path;
	char *_artist;
	char *_album;
	int _year;
	char *_comment;
	int _track;
	char *_genre;

	int audioProperties;
	int _bitrate;
	int _sample_rate;
	int _channels;
	int _length;

protected:
    TagNode(char *path);

    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> Read(const v8::Arguments& args);
    static v8::Handle<v8::Value> Path(const v8::Arguments& args);
    //static v8::Handle<v8::Value> Title(const v8::Arguments& args);
    //static v8::Handle<v8::Value> Artist(const v8::Arguments& args);
    static v8::Handle<v8::Value> Album(const v8::Arguments& args);
    static v8::Handle<v8::Value> Year(const v8::Arguments& args);
    static v8::Handle<v8::Value> Comment(const v8::Arguments& args);
    static v8::Handle<v8::Value> Track(const v8::Arguments& args);
    static v8::Handle<v8::Value> Genre(const v8::Arguments& args);

    // Your own object variables here

};

#endif
