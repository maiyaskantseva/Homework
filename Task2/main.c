#include <stdio.h>
#include <stdlib.h> 
#include "lodepng/lodepng.h"

int MAX_TANKER_SIZE = 6;

unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height) {
  unsigned char* image = NULL; 
  int error = lodepng_decode32_file(&image, width, height, filename);
  if (error != 0) {
    printf("error %u: %s\n", error, lodepng_error_text(error)); 
  }
  return (image);
}

void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
    unsigned char* png;
    long unsigned int pngsize;
    int error = lodepng_encode32(&png, &pngsize, image, width, height);
    if (error == 0) {
        lodepng_save_file(png, pngsize, filename);
    } else {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    free(png);
}

void contrast(unsigned char *col, int bw_size) {
    int i; 
    for (i = 0; i < bw_size; i++) {
        if (col[i] < 70) {
            col[i] = 0;
        } else if (col[i] < 85) {
            col[i] = 100;
        } else {
            col[i] = 255;
        }
    }
}

void convert_to_bw(unsigned char* bw_pic, unsigned char* pic, int size) {
    for (int i = 0; i < size; i += 4) {
        char r = pic[i];
        char g = pic[i + 1];
        char b = pic[i + 2];

        bw_pic[i / 4] = (r + g + b) / 3;
    }
}

void convert_to_rgba(unsigned char* bw_pic, unsigned char* pic, int size) {
    for (int i = 0; i < size; i += 4) {
        char gray = bw_pic[i / 4];

        char r = gray;
        char g = gray;
        char b = gray;
        char a = (char)255;

        pic[i] = r;
        pic[i + 1] = g;
        pic[i + 2] = b;
        pic[i + 3] = a;
    }
}

struct area {
    int x;
    int y;
    int width;
};

int dfs_size(int i, int j, int height, int width, unsigned char* bw_pic, int* used) {
    if (i < 0 || i >= height) {
        return 0;
    }
    if (j < 0 || j >= width) {
        return 0;
    }
    if (bw_pic[i * width + j] == 255 || used[i * width + j] == 1) {
        return 0;
    }

    used[i * width + j] = 1;

    int ans = 1;

    ans += dfs_size(i + 1, j, height, width, bw_pic, used);
    ans += dfs_size(i - 1, j, height, width, bw_pic, used);
    ans += dfs_size(i, j + 1, height, width, bw_pic, used);
    ans += dfs_size(i, j - 1, height, width, bw_pic, used);

    return ans;
}

void dfs_color(int i, int j, int height, int width, unsigned char* bw_pic, int* big, int* used) {
    if (i < 0 || i >= height) {
        return;
    }
    if (j < 0 || j >= width) {
        return;
    }
    if (bw_pic[i * width + j] == 255 || used[i * width + j] == 1) {
        return;
    }

    used[i * width + j] = 1;
    big[i * width + j] = 1;

    dfs_color(i + 1, j, height, width, bw_pic, big, used);
    dfs_color(i - 1, j, height, width, bw_pic, big, used);
    dfs_color(i, j + 1, height, width, bw_pic, big, used);
    dfs_color(i, j - 1, height, width, bw_pic, big, used);
}

int dfs_size_ok(int i, int j, int height, int width, unsigned char* bw_pic, int* big, int* used, int* ok) {
    if (i < 0 || i >= height) {
        return 0;
    }
    if (j < 0 || j >= width) {
        return 0;
    }
    if (bw_pic[i * width + j] == 0 || used[i * width + j] == 1) {
        if (bw_pic[i * width + j] == 0 && big[i * width + j] == 1) {
            *ok = 1;
        }
        return 0;
    }

    used[i * width + j] = 1;

    int ans = 1;

    ans += dfs_size_ok(i + 1, j, height, width, bw_pic, big, used, ok);
    ans += dfs_size_ok(i - 1, j, height, width, bw_pic, big, used, ok);
    ans += dfs_size_ok(i, j + 1, height, width, bw_pic, big, used, ok);
    ans += dfs_size_ok(i, j - 1, height, width, bw_pic, big, used, ok);

    return ans;
}

struct point {
    int x, y;
};
  
int main() {
    const char* filename = "tankers.png";
    unsigned int width, height;

    int size;
    int bw_size;

    unsigned char* picture = load_png(filename, &width, &height);
    if (picture == NULL) {
        printf("Problem reading picture from the file %s. Error.\n", filename); 
        return -1; 
    }

    write_png("readed.png", picture, width, height);

    size = width * height * 4;
    bw_size = width * height;
    
    
    unsigned char* bw_pic = (unsigned char*)malloc(bw_size*sizeof(unsigned char));

    convert_to_bw(bw_pic, picture, size);

    convert_to_rgba(bw_pic, picture, size);
    write_png("black_white.png", picture, width, height);

    contrast(bw_pic, bw_size);

    convert_to_rgba(bw_pic, picture, size);
    write_png("contrast.png", picture, width, height);

    int* used = (int*)malloc(bw_size * sizeof(int));
    int* big = (int*)malloc(bw_size * sizeof(int));

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            used[i * width + j] = 0;
            big[i * width + j] = 0;
        }
    }

    struct area max_1 = {0, 0, 0};
    struct area max_2 = {0, 0, 0};

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (!used[i * width + j] && bw_pic[i * width + j] == 0) {
                int sz = dfs_size(i, j, height, width, bw_pic, used);
                if (sz >= max_1.width) {
                    max_2 = max_1;
                    max_1.x = i;
                    max_1.y = j;
                    max_1.width = sz;
                } else if (sz >= max_2.width) {
                    max_2.x = i;
                    max_2.y = j;
                    max_2.width = sz;
                }
            }
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            used[i * width + j] = 0;
        }
    }

    dfs_color(max_1.x, max_1.y, height, width, bw_pic, big, used);
    dfs_color(max_2.x, max_2.y, height, width, bw_pic, big, used);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            used[i * width + j] = 0;
        }
    }

    int tanker = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (!used[i * width + j] && bw_pic[i * width + j] == 255) {
                int* ok = (int*)malloc(sizeof(int));
                int size = dfs_size_ok(i, j, height, width, bw_pic, big, used, ok);

                if (size <= MAX_TANKER_SIZE && *ok == 1) {
                    tanker++;
                }
            }
        }
    }

    printf("tankers count %d\n", tanker);

    free(bw_pic);
    free(picture); 
    
    return 0; 
}
