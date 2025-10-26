#pragma once

#ifdef USE_X68000
void fillboxx(int x1, int y1, int w, int h, COLORREF color);
void filltrapezoidx(int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color);
#else
void DirectFillRectangle(HDC hdc, int x1, int y1, int w, int h, COLORREF color);
void DirectFillTrapezoid(HDC hdc, int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color);
#endif

void DirectFillRectangleX(HDC hdc, int x1, int y1, int w, int h, COLORREF color) {
#ifdef USE_X68000
    fillboxx(x1, y1, w + 1, h + 1, color);
#else
    DirectFillRectangle(hdc, x1, y1, w + 1, h + 1, color);
#endif
}

void DirectFillTrapezoidX(HDC hdc, int x1, int y1, int x2, int y2, int w1, int w2, COLORREF color) {
#ifdef USE_X68000
    filltrapezoidx(x1, y1, x2, y2, w1, w2, color);
#else
    DirectFillTrapezoid(hdc, x1, y1, x2, y2, w1, w2, color);
#endif
}

void draw_digit1(HDC hdc, int n, int x, int y, int w, int h, COLORREF color) {
    double rw1 = 0.15;
    double rh1 = 0.1;
    double rw2 = 0.15;
    double rh2 = 0.2;
    double rx1 = 0.5;
    double rx3 = 0.1;
    double rx4 = 0.6;
    double rx7 = 0.3;
    double ry4 = 0.7;
    if (n == 0 || n == 2 || n == 3 || n == 6 || n == 8 || n == 9) {
        // ã‚Ì‰¡ü
        DirectFillTrapezoidX(hdc,
            (int)(x + w * rw1),
            (int)(y),
            (int)(x),
            (int)(y + h * rh1),
            (int)(w - w * rw1 * 2),
            (int)(w),
            color);
    }
    if (n == 0 || n == 3 || n == 5 || n == 6 || n == 8 || n == 9) {
        // ‰º‚Ì‰¡ü
        DirectFillTrapezoidX(hdc,
            (int)(x),
            (int)(y + h - h * rh1),
            (int)(x + w * rw1),
            (int)(y + h),
            (int)(w),
            (int)(w - w * rw1 * 2),
            color);
    }
    if (n == 3) {
        // ’†‰›‚Ì‰¡ü
        DirectFillTrapezoidX(hdc,
            (int)(x + w * rx3),
            (int)(y + h / 2 - h * rh1 * 0.5),
            (int)(x + w * rx3),
            (int)(y + h / 2),
            (int)(w - w * rx3),
            (int)(w - w * rx3 - w * rw1),
            color);
        DirectFillTrapezoidX(hdc,
            (int)(x + w * rx3),
            (int)(y + h / 2),
            (int)(x + w * rx3),
            (int)(y + h / 2 + h * rh1 * 0.5),
            (int)(w - w * rx3 - w * rw1),
            (int)(w - w * rx3),
            color);
    }
    if (n == 8) {
        // ’†‰›‚Ì‰¡ü
        DirectFillTrapezoidX(hdc,
            (int)(x),
            (int)(y + h / 2 - h * rh1 * 0.5),
            (int)(x + w * rw1),
            (int)(y + h / 2),
            (int)(w),
            (int)(w - w * rw1 * 2),
            color);
        DirectFillTrapezoidX(hdc,
            (int)(x + w * rw1),
            (int)(y + h / 2),
            (int)(x),
            (int)(y + h / 2 + h * rh1 * 0.5),
            (int)(w - w * rw1 * 2),
            (int)(w),
            color);
    }
    if (n == 5 || n == 6) {
        // ’†‰›‚Ì‰¡ü
        DirectFillTrapezoidX(hdc,
            (int)(x),
            (int)(y + h / 2 - h * rh1 * 0.5),
            (int)(x),
            (int)(y + h / 2 + h * rh1 * 0.5),
            (int)(w - w * rw1),
            (int)(w),
            color);
    }
    if (n == 9) {
        // ’†‰›‚Ì‰¡ü
        DirectFillTrapezoidX(hdc,
            (int)(x),
            (int)(y + h / 2 - h * rh1 * 0.5),
            (int)(x + w * rw1),
            (int)(y + h / 2 + h * rh1 * 0.5),
            (int)(w),
            (int)(w - w * rw1),
            color);
    }
    if (n == 0 || n == 6) {
        // ¶‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h * rh1),
            (int)(w * rw1),
            (int)(h - h * rh1 * 2),
            color);
    }
    if (n == 0 || n == 9) {
        // ‰E‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * rh1),
            (int)(w * rw1),
            (int)(h - h * rh1 * 2),
            color);
    }
    if (n == 1) {
        // cü
        DirectFillTrapezoidX(hdc,
            (int)(x + w * rx1 - w * rw1 / 2),
            (int)(y),
            (int)(x + w * rx1 - w * rw1 / 2 - w * rw2),
            (int)(y + h * rh1),
            (int)(0),
            (int)(w * rw2),
            color);
        DirectFillRectangleX(hdc,
            (int)(x + w * rx1 - w * rw1 / 2),
            (int)(y),
            (int)(w * rw1),
            (int)(h),
            color);
    }
    if (n == 4) {
        // cü
        DirectFillRectangleX(hdc,
            (int)(x + w * rx4 - w * rw1 / 2),
            (int)(y),
            (int)(w * rw1),
            (int)(h),
            color);
    }
    if (n == 2) {
        // ‰º‚Ì‰¡ü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h - h * rh1),
            (int)(w),
            (int)(h * rh1),
            color);
    }
    if (n == 4) {
        // ã‚Ì‰¡ü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h * ry4 - h * rh1 * 0.5),
            (int)(w),
            (int)(h * rh1),
            color);
    }
    if (n == 5 || n == 7) {
        // ã‚Ì‰¡ü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y),
            (int)(w),
            (int)(h * rh1),
            color);
    }
    if (n == 5 || n == 8 || n == 9) {
        // ¶ã‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h * rh1),
            (int)(w * rw1),
            (int)(h / 2 - h * rh1 * 1.5),
            color);
    }
    if (n == 3 || n == 8) {
        // ‰Eã‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * rh1),
            (int)(w * rw1),
            (int)(h / 2 - h * rh1 * 1.5),
            color);
    }
    if (n == 8) {
        // ¶‰º‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h / 2 + h * rh1 * 0.5),
            (int)(w * rw1),
            (int)(h / 2 - h * rh1 * 1.5),
            color);
    }
    if (n == 3 || n == 5 || n == 6 || n == 8) {
        // ‰E‰º‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h / 2 + h * rh1 * 0.5),
            (int)(w * rw1),
            (int)(h / 2 - h * rh1 * 1.5),
            color);
    }
    if (n == 2 || n == 3) {
        // ¶ã‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h * rh1),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 2 || n == 6 || n == 7) {
        // ‰Eã‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * rh1),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 3 || n == 5 || n == 9) {
        // ¶‰º‚Ìcü
        DirectFillRectangleX(hdc,
            (int)(x),
            (int)(y + h - h * rh1 - h * rh2),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 2) {
        // ŽÎ‚ß‚Ìü
        DirectFillTrapezoidX(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * rh1 + h * rh2),
            (int)(x),
            (int)(y + h - h * rh1),
            (int)(w * rw1),
            (int)(w * rw1),
            color);
    }
    if (n == 4) {
        // ŽÎ‚ß‚Ìü
        DirectFillTrapezoidX(hdc,
            (int)(x + w * rx4 - w * rw1 * 0.5),
            (int)(y),
            (int)(x),
            (int)(y + h * ry4 - h * rh1 * 0.5),
            (int)(w * rw1),
            (int)(w * rw1),
            color);
    }
    if (n == 7) {
        // ŽÎ‚ß‚Ìü
        DirectFillTrapezoidX(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * rh1 + h * rh2),
            (int)(x + w * rx7 - w * rw1 * 0.5),
            (int)(y + h),
            (int)(w * rw1),
            (int)(w * rw1),
            color);
    }
    if (n == 10) {
        // ¬”“_
        DirectFillEllipse(hdc,
            (int)(x + w * rx1 - w * rw1 / 2),
            (int)(y + h - h * rh1),
            (int)(w * rw1),
            (int)(h * rh1),
            color);
    }
    if (n == 11) {
        // ƒ}ƒCƒiƒX‚Ì•„†
        DirectFillRectangleX(hdc,
            (int)(x + w * rw1),
            (int)(y + h / 2 - h * rh1 * 0.5),
            (int)(w - w * rw1 * 2),
            (int)(h * rh1),
            color);
    }
}

void digital_vbar(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    double rh1 = 0.1;
    double rw1 = 0.5;
    DirectFillTrapezoidX(hdc,
        (int)(x + w * rw1),
        (int)(y),
        (int)(x),
        (int)(y + h * rh1),
        (int)(0),
        (int)(w),
        color);
    DirectFillRectangleX(hdc,
        (int)(x),
        (int)(y + h * rh1),
        (int)(w),
        (int)(h - h * rh1 * 2),
        color);
    DirectFillTrapezoidX(hdc,
        (int)(x),
        (int)(y + h - h * rh1),
        (int)(x + w * rw1),
        (int)(y + h),
        (int)(w),
        (int)(0),
        color);
}

void digital_hbar(HDC hdc, int x, int y, int w, int h, COLORREF color) {
    double rw1 = 0.1;
    double rh1 = 0.5;
    DirectFillTrapezoidX(hdc,
        (int)(x + w * rw1),
        (int)(y),
        (int)(x),
        (int)(y + h * rh1),
        (int)(w - w * rw1 * 2),
        (int)(w),
        color);
    DirectFillTrapezoidX(hdc,
        (int)(x),
        (int)(y + h * rh1),
        (int)(x + w * rw1),
        (int)(y + h),
        (int)(w),
        (int)(w - w * rw1 * 2),
        color);
}

void draw_digit2(HDC hdc, int n, int x, int y, int w, int h, COLORREF color) {
    double rw1 = 0.15;
    double rw2 = 1 - rw1;
    double rx1 = rw1 * 0.5;
    double rxp = 0.5;
    double rh1 = 0.1;
    double rh2 = 0.5 - rh1 * 0.5;
    double ry1 = rh1 * 0.5;
    double ry2 = ry1 + rh2;
    if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9) {
        // ã‚Ì‰¡ü
        digital_hbar(hdc,
            (int)(x + w * rx1),
            (int)(y),
            (int)(w * rw2),
            (int)(h * rh1),
            color);
    }
    if (n == 2 || n == 3 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9 || n == 11) {
        // ’†‰›‚Ì‰¡ü
        digital_hbar(hdc,
            (int)(x + w * rx1),
            (int)(y + h * rh2),
            (int)(w * rw2),
            (int)(h * rh1),
            color);
    }
    if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 8 || n == 9) {
        // ‰º‚Ì‰¡ü
        digital_hbar(hdc,
            (int)(x + w * rx1),
            (int)(y + h * rh2 * 2),
            (int)(w * rw2),
            (int)(h * rh1),
            color);
    }
    if (n == 0 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9) {
        // ¶ã‚Ìcü
        digital_vbar(hdc,
            (int)(x),
            (int)(y + h * ry1),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 0 || n == 1 || n == 2 || n == 3 || n == 4 || n == 7 || n == 8 || n == 9) {
        // ‰Eã‚Ìcü
        digital_vbar(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * ry1),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 0 || n == 2 || n == 6 || n == 8) {
        // ¶‰º‚Ìcü
        digital_vbar(hdc,
            (int)(x),
            (int)(y + h * ry2),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 0 || n == 1 || n == 3 || n == 4 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9) {
        // ‰E‰º‚Ìcü
        digital_vbar(hdc,
            (int)(x + w - w * rw1),
            (int)(y + h * ry2),
            (int)(w * rw1),
            (int)(h * rh2),
            color);
    }
    if (n == 10) {
        // ¬”“_
        DirectFillRectangleX(hdc,
            (int)(x + w * rxp - w * rw1 / 2),
            (int)(y + h - h * rh1),
            (int)(w * rw1),
            (int)(h * rh1),
            color);
    }
}

void draw_num_str(HDC hdc, const char* num_str, int numer_string_type, int x, int y, int w, int h, COLORREF color) {
    double r = 0.9;
    double w2 = w * r;
    double h2 = h * r;
    double xm = w * (1 - r) * 0.5;
    double ym = h * (1 - r) * 0.5;
    for (int i = 0; num_str[i] != '\0'; ++i) {
        char c = num_str[i];
        int n;
        if (c >= '0' && c <= '9') {
            n = c - '0';
        }
        else if (c == '.') {
            n = 10;
        }
        else if (c == '-') {
            n = 11;
        }
        else {
            continue; // •s–¾‚È•¶Žš‚Í–³Ž‹
        }
        if (numer_string_type == 0) {
            draw_digit1(hdc, n, x + w * i + xm, y + ym, w2, h2, color);
        }
        else if (numer_string_type == 1) {
            draw_digit2(hdc, n, x + w * i + xm, y + ym, w2, h2, color);
        }
    }
}

