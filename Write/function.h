#pragma once
#include "../libpotrace/potracelib.h"
#include <opencv2/opencv.hpp>
// using namespace cv;
//名字冲突会导致 “ACCESS_MASK“不明确的符号 ，故使用cv::代替
#ifdef _DEBUG
#pragma comment(lib, "../x64/Debug/libpotrace.lib")
#else
#pragma comment(lib, "../x64/Release/libpotrace.lib")
#endif // DEBUG

#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8 * BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1) << (BM_WORDBITS - 1))
#define bm_scanline(bm, y) ((bm)->map + (y) * (bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x) / BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS - 1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)

/*---------------------------------------------------------------------*/

/* calculate the size, in bytes, required for the data area of a
   bitmap of the given dy and h. Assume h >= 0. Return -1 if the size
   does not fit into the ptrdiff_t type. */
ptrdiff_t getsize(int dy, int h)
{
    ptrdiff_t size;

    if (dy < 0)
    {
        dy = -dy;
    }

    size = (ptrdiff_t)dy * (ptrdiff_t)h * (ptrdiff_t)BM_WORDSIZE;

    /* check for overflow error */
    if (size < 0 || (h != 0 && dy != 0 && size / h / dy != BM_WORDSIZE))
    {
        return -1;
    }

    return size;
}

/* return new bitmap initialized to 0. NULL with errno on error.
   Assumes w, h >= 0. */
potrace_bitmap_t *bm_new(int w, int h)
{
    potrace_bitmap_t *bm;
    int dy = w == 0 ? 0 : (w - 1) / BM_WORDBITS + 1;
    ptrdiff_t size;

    size = getsize(dy, h);
    if (size < 0)
    {
        errno = ENOMEM;
        return NULL;
    }
    if (size == 0)
    {
        size = 1; /* make sure calloc() doesn't return NULL */
    }

    bm = (potrace_bitmap_t *)malloc(sizeof(potrace_bitmap_t));
    if (!bm)
    {
        return NULL;
    }
    bm->w = w;
    bm->h = h;
    bm->dy = dy;
    bm->map = (potrace_word *)calloc(1, size);
    if (!bm->map)
    {
        free(bm);
        return NULL;
    }
    return bm;
}

/* free a bitmap */
void bm_free(potrace_bitmap_t *bm)
{
    if (bm != NULL)
    {
        free(bm->map);
    }
    free(bm);
}

// only for CV_8UC1 binary or grayscale image
//可能要将image.cols都乘以3（如果出现生成的eps残缺的问题）
potrace_bitmap_t *bitmapFromMat(const cv::Mat &image, int threshold)
{
    potrace_bitmap_t *bitmap = bm_new(image.cols, image.rows);
    int pi = 0;
    for (int row = 0; row < image.rows; ++row)
    {
        const uchar *ptr = image.ptr<uchar>(image.rows - 1 - row);
        for (int col = 0; col < image.cols; ++col)
        {
            if (ptr[col] > threshold)
                BM_PUT(bitmap, col, row, 0);
            else
                BM_PUT(bitmap, col, row, 1);
        }
    }
    return bitmap;
}
// bmp转为eps
int potrace_bmp(string FileName)
{
    int x, y, i;
    potrace_bitmap_t *bm;
    potrace_param_t *param;
    potrace_path_t *p;
    potrace_state_t *st;
    int n, *tag;
    potrace_dpoint_t(*c)[3];
    string PureFileName = FileName.substr(0, FileName.rfind("."));

    /* load a bitmap */
    cv::Mat cvbm = cv::imread(FileName, CV_8UC1);
    int Width = cvbm.cols;
    int Height = cvbm.rows;
    bm = bitmapFromMat(cvbm, 127);
    if (!bm)
    {
        fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno));
        return 1;
    }

    /* set tracing parameters, starting from defaults */
    param = potrace_param_default();
    if (!param)
    {
        fprintf(stderr, "Error allocating parameters: %s\n", strerror(errno));
        return 1;
    }
    param->turdsize = 0;

    /* trace the bitmap */
    st = potrace_trace(param, bm);
    if (!st || st->status != POTRACE_STATUS_OK)
    {
        fprintf(stderr, "Error tracing bitmap: %s\n", strerror(errno));
        return 1;
    }
    bm_free(bm);

    /* open file to write vector graphics content */
    // FILE* file = fopen("../output.eps", "w");
    FILE *file = fopen((PureFileName + ".eps").data(), "w");
    if (!file)
        return -1;

    /* output vector data, e.g. as a rudimentary EPS file */
    fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf(file, "%%%%BoundingBox: 0 0 %d %d\n", Width, Height);
    fprintf(file, "gsave\n");

    /* draw each curve */
    p = st->plist;
    while (p != NULL)
    {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        fprintf(file, "%f %f moveto\n", c[n - 1][2].x, c[n - 1][2].y);
        for (i = 0; i < n; i++)
        {
            switch (tag[i])
            {
            case POTRACE_CORNER:
                fprintf(file, "%f %f lineto\n", c[i][1].x, c[i][1].y);
                fprintf(file, "%f %f lineto\n", c[i][2].x, c[i][2].y);
                break;
            case POTRACE_CURVETO:
                fprintf(file, "%f %f %f %f %f %f curveto\n",
                        c[i][0].x, c[i][0].y,
                        c[i][1].x, c[i][1].y,
                        c[i][2].x, c[i][2].y);
                break;
            }
        }
        /* at the end of a group of a positive path and its negative
           children, fill. */
        if (p->next == NULL || p->next->sign == '+')
        {
            fprintf(file, "0 setgray fill\n");
        }
        p = p->next;
    }
    fprintf(file, "grestore\n");
    fprintf(file, "%%EOF\n");

    potrace_state_free(st);
    potrace_param_free(param);

    return 0;
}

// bmp转为eps重载
int potrace_bmp(cv::Mat cvbm)
{
    int x, y, i;
    potrace_bitmap_t *bm;
    potrace_param_t *param;
    potrace_path_t *p;
    potrace_state_t *st;
    int n, *tag;
    potrace_dpoint_t(*c)[3];

    /* load a bitmap */
    int Width = cvbm.cols;
    int Height = cvbm.rows;
    bm = bitmapFromMat(cvbm, 127);
    if (!bm)
    {
        fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno));
        return 1;
    }

    /* set tracing parameters, starting from defaults */
    param = potrace_param_default();
    if (!param)
    {
        fprintf(stderr, "Error allocating parameters: %s\n", strerror(errno));
        return 1;
    }
    param->turdsize = 0;

    /* trace the bitmap */
    st = potrace_trace(param, bm);
    if (!st || st->status != POTRACE_STATUS_OK)
    {
        fprintf(stderr, "Error tracing bitmap: %s\n", strerror(errno));
        return 1;
    }
    bm_free(bm);

    /* open file to write vector graphics content */
    FILE *file = fopen("output.eps", "w");
    if (!file)
        return -1;

    /* output vector data, e.g. as a rudimentary EPS file */
    fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
    fprintf(file, "%%%%BoundingBox: 0 0 %d %d\n", Width, Height);
    fprintf(file, "gsave\n");

    /* draw each curve */
    p = st->plist;
    while (p != NULL)
    {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        fprintf(file, "%f %f moveto\n", c[n - 1][2].x, c[n - 1][2].y);
        for (i = 0; i < n; i++)
        {
            switch (tag[i])
            {
            case POTRACE_CORNER:
                fprintf(file, "%f %f lineto\n", c[i][1].x, c[i][1].y);
                fprintf(file, "%f %f lineto\n", c[i][2].x, c[i][2].y);
                break;
            case POTRACE_CURVETO:
                fprintf(file, "%f %f %f %f %f %f curveto\n",
                        c[i][0].x, c[i][0].y,
                        c[i][1].x, c[i][1].y,
                        c[i][2].x, c[i][2].y);
                break;
            }
        }
        /* at the end of a group of a positive path and its negative
           children, fill. */
        if (p->next == NULL || p->next->sign == '+')
        {
            fprintf(file, "0 setgray fill\n");
        }
        p = p->next;
    }
    fprintf(file, "grestore\n");
    fprintf(file, "%%EOF\n");

    potrace_state_free(st);
    potrace_param_free(param);

    return 0;
}

// bmp转为svg
int potrace_bmp_to_svg(string FileName)
{
    int x, y, i;
    potrace_bitmap_t *bm;
    potrace_param_t *param;
    potrace_path_t *p;
    potrace_state_t *st;
    int n, *tag;
    potrace_dpoint_t(*c)[3];
    string PureFileName = FileName.substr(0, FileName.rfind("."));

    /* load a bitmap */
    cv::Mat cvbm = cv::imread(FileName, CV_8UC1);
    int Width = cvbm.cols;
    int Height = cvbm.rows;
    bm = bitmapFromMat(cvbm, 127);
    if (!bm)
    {
        fprintf(stderr, "Error allocating bitmap: %s\n", strerror(errno));
        return 1;
    }

    /* set tracing parameters, starting from defaults */
    param = potrace_param_default();
    if (!param)
    {
        fprintf(stderr, "Error allocating parameters: %s\n", strerror(errno));
        return 1;
    }
    param->turdsize = 0;

    /* trace the bitmap */
    st = potrace_trace(param, bm);
    if (!st || st->status != POTRACE_STATUS_OK)
    {
        fprintf(stderr, "Error tracing bitmap: %s\n", strerror(errno));
        return 1;
    }
    bm_free(bm);

    /* open file to write vector graphics content */
    // FILE* file = fopen("../output.eps", "w");
    FILE *file = fopen((PureFileName + ".svg").data(), "w");
    if (!file)
        return -1;

    /* output vector data, e.g. as a rudimentary svg file */
    // svg的头
    fprintf(file, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
    fprintf(file, "<!DOCTYPE svg PUBLIC \" -\/\/W3C\/\/DTD SVG 1.1\/\/EN\"\n");
    fprintf(file, "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    fprintf(file, "<svg width=\"%d\" height=\"%d\" version=\"1.1\"\n", Width, Height);
    fprintf(file, "xmlns=\"http://www.w3.org/2000/svg\">\n");

    /* draw each curve */
    p = st->plist;
    while (p != NULL)
    {
        n = p->curve.n;
        tag = p->curve.tag;
        c = p->curve.c;
        // 垂直方向上开始会倒过来，加一个Height - 正常
        fprintf(file, "<path d=\"\nM%f %f\n", c[n - 1][2].x, Height - c[n - 1][2].y);
        for (i = 0; i < n; i++)
        {
            switch (tag[i])
            {
            case POTRACE_CORNER:
                fprintf(file, "L%f %f\n", c[i][1].x, Height - c[i][1].y);
                fprintf(file, "L%f %f\n", c[i][2].x, Height - c[i][2].y);
                break;
            case POTRACE_CURVETO:
                fprintf(file, "C%f %f %f %f %f %f\n",
                        c[i][0].x, Height - c[i][0].y,
                        c[i][1].x, Height - c[i][1].y,
                        c[i][2].x, Height - c[i][2].y);
                break;
            }
        }
        //右引号
        fprintf(file, "\"\n");
        /* at the end of a group of a positive path and its negative
           children, fill. */
        if (p->next == NULL || p->next->sign == '+')
        {
            // path属性添加
            fprintf(file, "style=\"fill:white;stroke:red;stroke-width:2\"/>\n");
        }
        p = p->next;
    }
    // svg尾
    fprintf(file, "</svg>\n");

    potrace_state_free(st);
    potrace_param_free(param);

    return 0;
}
