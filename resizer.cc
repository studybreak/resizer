/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>
#include <magick/api.h>

#include <string.h>
#include <unistd.h>

using namespace node;
using namespace v8;

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a function")));  \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_NUM_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsInt32())                      \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a number")));    \
  int VAR = args[I]->Int32Value();

#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a string")));    \
  v8::String::AsciiValue VAR(args[I]);

class Resizer: ObjectWrap
{
public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target)
  {
    InitializeMagick("./");

    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    s_ct->SetClassName(String::NewSymbol("Resizer"));

    NODE_SET_PROTOTYPE_METHOD(s_ct, "resize", Resize);

    target->Set(String::NewSymbol("Resizer"),
                s_ct->GetFunction());
  }

  Resizer()
  {
  }

  ~Resizer()
  {
  }

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    Resizer* resizer = new Resizer();
    resizer->Wrap(args.This());
    return args.This();
  }

  struct resize_baton_t {
    Resizer *resizer;
    char* from;
    char* to;
    int width;
    int height;
    Persistent<Function> cb;
    char* err;
  };

  static Handle<Value> Resize(const Arguments& args)
  {
    HandleScope scope;

    REQ_STR_ARG(0, from);
    REQ_STR_ARG(1, to);
    REQ_NUM_ARG(2, width);
    REQ_NUM_ARG(3, height);
    REQ_FUN_ARG(4, cb);

    Resizer* resizer = ObjectWrap::Unwrap<Resizer>(args.This());

    resize_baton_t *baton = new resize_baton_t();
    baton->resizer = resizer;

    baton->from = new char[(from.length() + 1)];
    strcpy(baton->from, *from);

    baton->to = new char[(to.length() + 1)];
    strcpy(baton->to, *to);

    baton->width = width;
    baton->height = height;
    baton->cb = Persistent<Function>::New(cb);

    baton->err = NULL;

    resizer->Ref();

    eio_custom(EIO_Resize, EIO_PRI_DEFAULT, EIO_AfterResize, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }


  static int EIO_Resize(eio_req *req)
  {
    resize_baton_t *baton = static_cast<resize_baton_t *>(req->data);
    ExceptionInfo exception;
    ImageInfo *info = CloneImageInfo((ImageInfo *) NULL);
    Image *image = NULL;
    Image *thumb = NULL;

    GetExceptionInfo(&exception);

    printf("To %s From %s Size %dx%d\n", baton->to, baton->from, baton->width, baton->height);

    strcpy(info->filename, baton->from);

    image = ReadImage(info, &exception);
    if (!image)
      goto CLEANUP;

    thumb = ThumbnailImage(image, baton->width, baton->height, &exception);

    if (!thumb)
      goto CLEANUP;

    strcpy(info->filename, baton->to);
    WriteImage(info, thumb);

CLEANUP:
//    if (exception->error_number)
  //    baton->err = GetExceptionMessage(exception->error_number);
    if (image)
      DestroyImage(image);
    if (thumb)
      DestroyImage(thumb);
    DestroyImageInfo(info);
    DestroyExceptionInfo(&exception);

    return 0;
  }

  static int EIO_AfterResize(eio_req *req)
  {
    HandleScope scope;
    resize_baton_t *baton = static_cast<resize_baton_t *>(req->data);
    ev_unref(EV_DEFAULT_UC);
    baton->resizer->Unref();

    Local<Value> argv[1];

    argv[0] = String::New(baton->err ? baton->err : "");

    TryCatch try_catch;

    baton->cb->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    baton->cb.Dispose();

    delete baton->to;
    delete baton->from;
    delete baton;
    return 0;
  }

};

Persistent<FunctionTemplate> Resizer::s_ct;

extern "C" {
  static void init (Handle<Object> target)
  {
    Resizer::Init(target);
  }

  NODE_MODULE(resizer, init);
}
