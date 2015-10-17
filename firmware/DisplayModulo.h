#ifndef MODULO_COLOR_DISPLAY_H
#define MODULO_COLOR_DISPLAY_H

#include "BaseModulo.h"

#ifdef SPARK
#include "spark_wiring_print.h"
#else
#include "Print.h"
#endif

/// A tiny color OLED screen
class DisplayModulo : public BaseModulo, public Print {
public:
    virtual ~DisplayModulo();

    int width() const {
        return 96;
    }

    int height() const {
        return 64;
    }

    enum TextAlignment {
        Left,
        Center,
        Right
    };

    struct Color {
        Color() : r(0), g(0), b(0), a(0) {}
        Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) :
            r(r_), g(g_), b(b_), a(a_) {}

        uint8_t r, g, b, a;
    };

    static const Color Black;
    static const Color White;
    static const Color Clear;

    /// Use the first DisplayModulo that's not already in use.
    DisplayModulo();

    /// Use the DisplayModulo with the specified deviceID
    DisplayModulo(uint16_t deviceID);

    void clear();

    //void setRotation(uint8_t r);
    //void setCursor(int x, int y);
    void setLineColor(const Color &c);
    void setLineColor(uint8_t r, uint8_t g, uint8_t b) {
        setLineColor(Color(r,g,b));
    }

    void setFillColor(const Color &c);
    void setFillColor(uint8_t r, uint8_t g, uint8_t b) {
        setFillColor(Color(r,g,b));
    }

    void setTextColor(const Color &textColor);
    void setTextColor(uint8_t r, uint8_t g, uint8_t b) {
        setTextColor(Color(r,g,b));
    }

    void setTextSize(uint8_t size);

    void fillScreen(Color color);
    void fillScreen(uint8_t r, uint8_t g, uint8_t b) {
        fillScreen(Color(r,g,b));
    }

    void setCursor(int x, int y);

    void drawLine(int x0, int y0, int x1, int y1);

    void drawRect(int x, int y, int w, int h, int radius=0);

    void drawCircle(int x, int y, int r);

    //void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2);

    void drawString(const char *s);

    size_t write(uint8_t c);
    void refresh();

    bool isComplete();

    /// Return whether the specified button is currently pressed
    bool getButton(int button);

    /// Return the state of all three buttons, one in each bit.
    uint8_t getButtons();

    /// Draw the Modulo logo and the word 'MODULO' on a purple background
    void drawSplashScreen();

    /// Draw the Modulo logo
    void drawLogo(int x=0, int y=0, int width=49, int height=49);

    void _setCurrentOp(uint8_t opCode);
    void _appendToOp(uint8_t data);
    void _endPreviousOp();

private:
    static const int OP_BUFFER_SIZE = 30;

    uint8_t _currentOp;
    uint8_t _opBuffer[OP_BUFFER_SIZE];
    uint8_t _opBufferLen;



    // Helper for drawString. Draws string s of length len (no null byte)
    void _drawString(const char *s, int len);

    void _waitOnRefresh();

    bool _isRefreshing;

};

#endif