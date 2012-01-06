#ifndef TAGNODE_HPP
#define TAGNODE_HPP

//#include <iostream>
//#include <stdio.h>

#include <node.h>

#include <string.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>


// Do not include this line. It's generally frowned upon to use namespaces
// in header files as it may cause issues with other code that includes your
// header file.
// using namespace v8;


// Forward declaration.
void AsyncRead(uv_work_t* req);
void AsyncReadAfter(uv_work_t* req);

void AsyncWrite(uv_work_t* req);
void AsyncWriteAfter(uv_work_t* req);

// We use a struct to store information about the asynchronous "work request".
struct Baton;

class TagNode : public node::ObjectWrap {
public:
	~TagNode() { // TODO: Find out how to destroy & erase memory
/*		//FIXME: Is it correct to do this?
		if (_title != NULL)
			free(_title);
		if (_artist != NULL)
			free(_artist);
		if (_album != NULL)
			free(_album);
		if (_comment != NULL)
			free(_comment);
		if (_genre != NULL)
			free(_genre);
		free(_path);*/
	};

	static v8::Persistent<v8::FunctionTemplate> constructor;
	static void Init(v8::Handle<v8::Object> target);

	static v8::Handle<v8::Value> GetTitle(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetTitle(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetArtist(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetArtist(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetAlbum(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetAlbum(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetYear(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetYear(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetComment(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetComment(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetTrack(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetTrack(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetGenre(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetGenre(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetBitrate(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetBitrate(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetSamplerate(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetSamplerate(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetChannels(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetChannels(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	static v8::Handle<v8::Value> GetLength(v8::Local<v8::String> property, const v8::AccessorInfo& info);
	static void SetLength(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

	//Vars made public, lazynees of coding setters
	char *_path;

	int _tag;
	char *_title;
	char *_artist;
	char *_album;
	int _year;
	char *_comment;
	int _track;
	char *_genre;

	int _audioProperties;
	int _bitrate;
	int _samplerate;
	int _channels;
	int _length;

	bool fTitle;// = false;
	bool fArtist;// = false;
	bool fAlbum;// = false;
	bool fYear;// = false;
	bool fComment;// = false;
	bool fTrack;// = false;
	bool fGenre;// = false;
	bool fBitrate;// = false;
	bool fSamplerate;// = false;
	bool fChannels;// = false;
	bool fLength;// = false;

protected:
    TagNode(char *path);

    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> Read(const v8::Arguments& args);
    static v8::Handle<v8::Value> Write(const v8::Arguments& args);
    static v8::Handle<v8::Value> Path(const v8::Arguments& args);
    static v8::Handle<v8::Value> Tag(const v8::Arguments& args);
    static v8::Handle<v8::Value> AudioProperties(const v8::Arguments& args);

    // Your own object variables here

};

#endif
