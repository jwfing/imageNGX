#ifndef _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGE_OPERATOR_H_
#define _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGE_OPERATOR_H_

#include <string>

class ImageOperator {
public:
    ImageOperator() {
        _fmt = "";
        _quality = -1;
        _hue = -1;
        _saturation = -1;
        _brightness = -1;
        _exposure = -101;
        _blur = -1;
        _crop = "";
        _rainbowWidth = -1;
        _rainbowHeight = -1;
        _rainbows = "";
    };
    virtual ~ImageOperator() {};

public:
    void setFmt(std::string fmt) {_fmt = fmt;};
    std::string getFmt() {return _fmt;};

    void setQuality(int quality) {_quality = quality;};
    int getQuality() {return _quality;};

    void setSaturation(double saturation) {_saturation = saturation;};
    double getSaturation() {return _saturation;};

    void setHue(double hue) {_hue = hue;};
    double getHue() {return _hue;};

    void setBrightness(double brightness) {_brightness = brightness;};
    double getBrightness() {return _brightness;};

    void setExposure(double exposure) {_exposure = exposure;};
    double getExposure() {return _exposure;};

    void setBlur(double blur) {_blur = blur;};
    double getBlur() {return _blur;};

    void setCrop(std::string crop) {_crop = crop;};
    std::string getCrop() {return _crop;};

    void setRainbowWidth(double rainbowWidth) {_rainbowWidth = rainbowWidth;};
    double getRainbowWidth() {return _rainbowWidth;};

    void setRainbowHeight(double rainbowHeight) {_rainbowHeight = rainbowHeight;};
    double getRainbowHeight() {return _rainbowHeight;};

    void setRainbows(const std::string& rainbows) {_rainbows = rainbows;};

public:
    virtual void doRun(void* image) = 0;

protected:
    std::string _fmt;
    std::string _crop;
    int _quality;
    double _hue;
    double _saturation;
    double _brightness;
    double _exposure;
    double _blur;
    std::string _rainbows;
    double _rainbowHeight;
    double _rainbowWidth;
};

#endif // end of _CN_LEANCLOUD_BACKEND_IMAGE_SERVICE_IMAGE_OPERATOR_H_
