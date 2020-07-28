#include <napi.h>
#include <list>
#include <Magick++.h>

using namespace std;
using namespace Magick;

class MemeWorker : public Napi::AsyncWorker {
 public:
  MemeWorker(Napi::Function& callback, string in_path, string text_top, string text_bottom, string type, int delay)
      : Napi::AsyncWorker(callback), in_path(in_path), text_top(text_top), text_bottom(text_bottom), type(type), delay(delay) {}
  ~MemeWorker() {}

  void Execute() {
    list<Image> frames;
    list<Image> coalesced;
    list<Image> mid;
    Image top_text;
    Image bottom_text;
    readImages(&frames, in_path);
    coalesceImages(&coalesced, frames.begin(), frames.end());
    for_each(coalesced.begin(), coalesced.end(), scaleImage(Geometry(600, 600)));

    top_text.size(Geometry(to_string(coalesced.front().columns())));
    top_text.backgroundColor("none");
    top_text.font("Impact");
    top_text.fontPointsize(40);
    top_text.textGravity(Magick::CenterGravity);
    top_text.read("pango:<span foreground='white'>" + text_top + "</span>");
    Image top_text_fill = top_text;
    top_text_fill.channel(Magick::AlphaChannel);
    top_text_fill.morphology(Magick::EdgeOutMorphology, "Octagon");
    top_text_fill.backgroundColor("black");
    top_text_fill.alphaChannel(Magick::ShapeAlphaChannel);
    top_text.composite(top_text_fill, Magick::CenterGravity, Magick::DstOverCompositeOp);

    if (text_bottom != "") {
      bottom_text.size(Geometry(to_string(coalesced.front().columns())));
      bottom_text.backgroundColor("none");
      bottom_text.font("Impact");
      bottom_text.fontPointsize(40);
      bottom_text.textGravity(Magick::CenterGravity);
      bottom_text.read("pango:<span foreground='white'>" + text_bottom + "</span>");
      Image bottom_text_fill = bottom_text;
      bottom_text_fill.channel(Magick::AlphaChannel);
      bottom_text_fill.morphology(Magick::EdgeOutMorphology, "Octagon");
      bottom_text_fill.backgroundColor("black");
      bottom_text_fill.alphaChannel(Magick::ShapeAlphaChannel);
      bottom_text.composite(bottom_text_fill, Magick::CenterGravity, Magick::DstOverCompositeOp);
    }

    for (Image &image : coalesced) {
      image.composite(top_text, Magick::NorthGravity, Magick::OverCompositeOp);
      if (text_bottom != "") image.composite(bottom_text, Magick::SouthGravity, Magick::OverCompositeOp);
      image.magick(type);
      mid.push_back(image);
    }

    if (delay != 0) for_each(mid.begin(), mid.end(), animationDelayImage(delay));
    writeImages(mid.begin(), mid.end(), &blob);
  }

  void OnOK() {
    Callback().Call({Env().Undefined(), Napi::Buffer<char>::Copy(Env(), (char *)blob.data(), blob.length())});
  }

 private:
  string in_path, type, text_top, text_bottom;
  int delay;
  Blob blob;
};

Napi::Value Meme(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();

  string in_path = info[0].As<Napi::String>().Utf8Value();
  string text_top = info[1].As<Napi::String>().Utf8Value();
  string text_bottom = info[2].As<Napi::String>().Utf8Value();
  string type = info[3].As<Napi::String>().Utf8Value();
  int delay = info[4].As<Napi::Number>().Int32Value();
  Napi::Function cb = info[5].As<Napi::Function>();

  MemeWorker* blurWorker = new MemeWorker(cb, in_path, text_top, text_bottom, type, delay);
  blurWorker->Queue();
  return env.Undefined();
}