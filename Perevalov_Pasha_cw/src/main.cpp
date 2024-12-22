#include <iostream>
#include <fstream>
#include <getopt.h>
#include <png.h>
#include <regex>
#include <string.h>
#include <cmath>
#include <vector>
#include <map>

#define CW "Course work for option 4.16, created by Pasha Perevalov\n"
#define H "--square - рисование квадрата, --exchange - поменять 4 области местами, --freq_color - замена самого часто встречаемого цвета\n"

#define signature_size 8 // размер сигнатуры PNG-файла (8 байт)

typedef struct{
	int R;
	int G;
	int B;
}Rgb;

static struct option keys[] = {
	{"help", no_argument, 0, 'h'},
	{"output", required_argument, 0, 'o'},
	{"input", required_argument, 0, 'i'},
	{"info", no_argument, 0, 'd'},
	{"square", no_argument, 0, 's'},
	{"exchange", no_argument, 0, 'e'},
	{"freq_color", no_argument, 0, 'y'},
	{"left_up", required_argument, 0, 'u'},
    {"right_down", required_argument, 0, 'p'},
	{"side_size", required_argument, 0, 'q'},
	{"thickness", required_argument, 0, 'w'},
	{"color", required_argument, 0, 'z'},
	{"fill", no_argument, 0, 'f'},
	{"fill_color", required_argument, 0, 'g'},
	{"exchange_type", required_argument, 0, 't'},
	{"radius", required_argument, 0, 'l'},
	{0, 0, 0, 0}
};

static struct option selectAction[] = {
	{"help", no_argument, 0, 'h'},
	{"output", required_argument, 0, 'o'},
	{"input", required_argument, 0, 'i'},
	{"info", no_argument, 0, 'd'},
	{"square", no_argument, 0, 'r'},
	{"exchange", no_argument, 0, 'x'},
	{"freq_color", no_argument, 0, 'c'},
	{0, 0, 0, 0}
};

static struct option squareKeys[] = {
	{"left_up", required_argument, 0, 'u'},
	{"side_size", required_argument, 0, 'd'},
	{"thickness", required_argument, 0, 't'},
	{"color", required_argument, 0, 'c'},
	{"fill", no_argument, 0, 'f'},
	{"fill_color", required_argument, 0, 'g'},
	{0, 0, 0, 0}
};

static struct option exchangeKeys[] = {
	{"exchange", no_argument, 0, 'd'},
	{"right_down", required_argument, 0, 'w'},
	{"left_up", required_argument, 0, 'z'},
	{"exchange_type", required_argument, 0, 'y'},
	{0, 0, 0, 0}
};

static struct option freqKeys[] = {
	{"color", required_argument, 0, 'l'},
	{0, 0, 0, 0}
};

// имя файла
bool checkValidName(std::string name){
	return(std::regex_match(name, std::regex("((./)||((\\w+/)+))?(\\w+).(\\w+)")));
}

bool findUnknownKey(int argc, char *argv[]){
	int keyIndex;
	int opt;
	char** argvCopy = new char*[argc]; // Создаётся копия массива argv, чтобы не изменять ориг массив.
    for (int i = 0; i < argc; ++i) { 
        size_t len = strlen(argv[i]) + 1;
        argvCopy[i] = new char[len];
        strncpy(argvCopy[i], argv[i], len);
    }
	while(true){
		opt = getopt_long(argc, argvCopy, "io:h", keys, &keyIndex);
		if(opt == -1){
			opt = 0;
			break;
		}
		switch(opt){
			case '?': // Если найден неизвестный ключ
				optind = 1; // Сброс индекса на начало аргументов
				return true;
				break;
		}
	}
	optind = 1; // чтобы не повлиять на последующие вызовы getopt_long
	return false;
}

void outputHelp(){
   std::cout << H << std::endl;
}

void outputInfo(int &height, int &width){
    std::cout << "Image height: " << height << ", image width: " << width << std::endl;
}

bool isPNG(const std::string& filename) {
    const std::vector<unsigned char> pngSignature = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    std::ifstream file(filename, std::ios::binary); // функция открывает файл в бинарном режиме.
    if (!file.is_open()) {
        std::cout << "Can't open file: " << filename << std::endl;
        exit(41);
        return false;
    }
    std::vector<unsigned char> fileSignature(8);
    file.read(reinterpret_cast<char*>(fileSignature.data()), fileSignature.size());
    if (file.gcount() != 8) {
        std::cout << "File too small: " << filename << std::endl;
        exit(41);
        return false;
    }
    return fileSignature == pngSignature;
}

bool convertCoords(std::string stringCoords, int &x, int &y){
	if(!std::regex_match(stringCoords.c_str(), std::regex("(-?[0-9]+).(-?[0-9]+)"))) return false;
	x = atoi(stringCoords.c_str()); // находим х
	int i = 0;
	while(true){
		if(stringCoords[i] == '.') break;
		i++;
	}
	y = atoi(i + 1 + stringCoords.c_str()); // находим y
	return true;
}

bool read_png_file(std::string filename, png_bytep* &new_png, int &height, int &width, png_byte &color_type, png_byte &bit_depth){

    if(isPNG(filename.c_str()) == false) return false;

    FILE *fp = fopen(filename.c_str(), "rb");


    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) abort();

    png_infop info = png_create_info_struct(png); // создание структуры информации о PNG
    if(!info) abort();

    if(setjmp(png_jmpbuf(png))) abort(); // установка обработчика ошибок

    png_init_io(png, fp); // инициализируется поток ввода для чтения PNG-данных из файла fp

    png_read_info(png, info);

	// Извлекаются характеристики изображения
    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth = png_get_bit_depth(png, info);
	
	// Обработка глубины цвета и типа цвета
    if(bit_depth == 16)
        png_set_strip_16(png);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if(png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    if (new_png) abort();

    new_png = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        new_png[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
    }

    if (!new_png) abort();

    png_read_image(png, new_png);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);

    return true;
}

void write_png_file(std::string filename, png_bytep* new_png, int height, int width){
    int y;

    FILE *fp = fopen(filename.c_str(), "wb");
    if(!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    png_set_IHDR(
        png,
        info,
        width, height,
        8,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    if (!new_png) abort();

    png_write_image(png, new_png);
    png_write_end(png, NULL);

    for(int y = 0; y < height; y++) {
        free(new_png[y]);
    }
    free(new_png);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

void set_pixel_color(int x, int y, int *color, png_bytep* png){
	png[y][(x * 4)] = color[0];
	png[y][(x * 4)+1] = color[1];
	png[y][(x * 4)+2] = color[2];
}

void getRgb(std::string &color, int* arrayRgb){
	char colorRgb[color.size()+1];
	char* pointer;
	strcpy(colorRgb, color.c_str());
	pointer = strtok(colorRgb, ".");
	arrayRgb[0] = atoi(pointer); // получаем красный
	pointer = strtok(NULL, ".");
	arrayRgb[1] = atoi(pointer); // зеленый
	pointer = strtok(NULL, ".");
	arrayRgb[2] = atoi(pointer); // синий
}

// проверка допустимых границ
bool checkValidCoord(int& x, int& y, int &height, int &width){
	if(x >= width || x < 0) return false;
	if(y >= height || y < 0) return false;
	return true;
}

void drawCircle(int& x, int& y, int thickness, std::string& color,
                png_bytep* &newPng, int &height, int &width){
    int r = thickness / 2;
    int arrayRgb[3];
    getRgb(color, arrayRgb); 
    if(r < 1){ 
		if(checkValidCoord(x, y, height, width)){
        	set_pixel_color(x, y, arrayRgb, newPng);
		} 
        return; 
    }
	// Используем алгоритм Брезенхема
    int xc = r;
    int yc = 0;
    int P = 1 - r;
    while (xc >= yc) {
		int xMinusXC  = x - xc;
		int xPlusXC = x + xc;
		int yPlusYC = y + yc;
		int yMinusYC = y - yc;
		int yPlusXC = y + xc;
		int xPlusYC = x + yc;
		int xMinusYC  = x - yc;
		int yMinusXC = y - xc;
		// Рисование горизонтальных и вертикальных линий
        for (int i = xMinusXC; (i <= xPlusXC)&&(i < width); i++) {
            if ((yPlusYC >= 0)&&(i >= 0)&&(yPlusYC < height)){
                set_pixel_color(i, yPlusYC, arrayRgb, newPng);
            }
            if ((yMinusYC >= 0)&&(i >= 0)&&(yMinusYC < height)) {
                set_pixel_color(i, yMinusYC, arrayRgb, newPng);
            }
        }
        for (int i = xMinusYC; (i <= xPlusYC)&&(i < width); i++) {
    		if ((yPlusXC >= 0)&&(i >= 0)&&(yPlusXC < height)) {
                set_pixel_color(i, yPlusXC, arrayRgb, newPng);
    		}
    		if ((yMinusXC >= 0)&&(i >= 0)&&(yMinusXC < height)) {
                set_pixel_color(i, yMinusXC, arrayRgb, newPng);
    		}
		}
		yc++;
		if (P <= 0) {
    		P = P + 2 * yc + 1;
		} else {
    		xc--;
    		P = P + 2 * (yc - xc) + 1;
		}
	}
}

void drawLine(int firstX, int firstY, int secondX, int secondY, int thickness, 
              std::string& color, png_bytep* &newPng, int &height, int &width){
    int dx = abs(secondX - firstX);
    int dy = abs(secondY - firstY);
    int sx = firstX < secondX ? 1 : -1;
    int sy = firstY < secondY ? 1 : -1;
    int err = dx - dy;
    int x = firstX;
    int y = firstY;
    while(x != secondX || y != secondY){ 
        drawCircle(x, y, thickness, color, newPng, height, width);
        int err2 = 2 * err;
        if(err2 > -dy){
            err -= dy;
            x += sx;
        }
        if(err2 < dx){
            err += dx;
            y += sy;
        }
    }
	drawCircle(secondX, secondY, thickness, color, newPng, height, width); // конечная точка линии
}

// прямоугольник
void fillSquare(int leftX, int leftY, int rightX, int rightY, int thickness, png_bytep* &newPng, std::string &fillColor, int &height, int &width){
	int arrayFillRgb[3];
	getRgb(fillColor, arrayFillRgb);
	for(int i = leftX; i <= rightX; i++){
		for(int k = leftY; k <= rightY; k++){
			if(checkValidCoord(i, k, height, width)){
                set_pixel_color(i, k, arrayFillRgb, newPng);
			}
		}
	}
}

void drawSquare(int leftX, int leftY, int side_size, int thickness,
              std::string& color, bool fill, std::string fillColor, png_bytep* &newPng, int &height, int &width){
    int rightX = leftX + side_size;
    int rightY = leftY + side_size;
	if(fill == true) fillSquare(leftX, leftY, rightX, rightY, thickness, newPng, fillColor, height, width);
	drawLine(leftX, leftY, rightX, leftY, thickness, color, newPng, height, width);
	drawLine(rightX, leftY, rightX, rightY, thickness, color, newPng, height, width);
	drawLine(leftX, rightY, rightX, rightY, thickness, color, newPng, height, width);
	drawLine(leftX, leftY, leftX, rightY, thickness, color, newPng, height, width);
}

void processSquareCommand(int &height, int &width, png_bytep* &newPng, int argc, char* argv[]){
	int opt;
	int keyIndex;
	int leftX = -1;
	int leftY = -1;
	int side_size = -1;
	int thickness = -1;
	std::string color = "";
	std::string fillColor = "";
	bool fill = false;
	while(true){
		opt = getopt_long(argc, argv, "", squareKeys, &keyIndex);
		if(opt == -1){
			opt = 0;			
			break;
		}
		switch(opt){
			case 'u':
				convertCoords(optarg, leftX, leftY);
				break;
			case 'd':
				side_size = atoi(optarg);
				break;
			case 't':
				if(std::regex_match(optarg, std::regex("([0-9]+)"))) thickness = atoi(optarg);
				break;
			case 'c':
				if(std::regex_match(optarg, std::regex
				("(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9]).(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9]).(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9])")))
					color = optarg;
				break;
			case 'f':
				fill = true;
				break;
			case 'g':
				if(std::regex_match(optarg, std::regex
				("(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9]).(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9]).(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9])")))
					fillColor = optarg;
				break;
		}
	}
	if(leftX == -1 || leftY == -1 || side_size == -1 || thickness == -1 || color == ""){
		std::cout << "Invalid data" << std::endl;
		exit(41);
		return;
	}
	if(fill == true && fillColor == ""){
		std::cout << "Invalid data" << std::endl;
		exit(41);
		return;
	}
	drawSquare(leftX, leftY, side_size, thickness, color, fill, fillColor, newPng, height, width);
}

void processExchangeCommand(int &height, int &width, png_bytep* & newPng, int argc, char* argv[]){
	int opt;
	int keyIndex;
	int leftX = -1;
	int leftY = -1;
	int rightX = -1;
	int rightY = -1;
	std::string exchangeType;
	while(true){
		opt = getopt_long(argc, argv, "", exchangeKeys, &keyIndex);
		if(opt == -1){
			opt = 0;			
			break;
		}
		switch(opt){
			case 'w':
				convertCoords(optarg, rightX, rightY);
				break;
			case 'z':
				convertCoords(optarg, leftX, leftY);
				break;
			case 'y':
				exchangeType = optarg;
				break;
		}
	}

	if(leftX < 0 || leftY < 0 || rightX < 0 || rightY < 0 || rightX > width || rightY > height){
		std::cout << "Invalid data" << std::endl;
		exit(41);
		return;
	}

	int midX = (leftX + rightX) / 2;
    int midY = (leftY + rightY) / 2;
    int exchange_width = rightX - leftX + 1;
    int exchange_height = rightY - leftY + 1;

    std::vector<png_bytep> topLeft(exchange_height / 2);
    std::vector<png_bytep> topRight(exchange_height / 2);
    std::vector<png_bytep> bottomLeft(exchange_height / 2);
    std::vector<png_bytep> bottomRight(exchange_height / 2);

    for (int i = 0; i < exchange_height / 2; ++i) {
        topLeft[i] = new png_byte[exchange_width / 2 * 4];
        topRight[i] = new png_byte[exchange_width / 2 * 4];
        bottomLeft[i] = new png_byte[exchange_width / 2 * 4];
        bottomRight[i] = new png_byte[exchange_width / 2 * 4];
    }

    for (int y = 0; y < exchange_height / 2; ++y) {
        std::copy(newPng[leftY + y] + leftX * 4, newPng[leftY + y] + midX * 4, topLeft[y]);
        std::copy(newPng[leftY + y] + midX * 4, newPng[leftY + y] + rightX * 4, topRight[y]);
        std::copy(newPng[midY + y] + leftX * 4, newPng[midY + y] + midX * 4, bottomLeft[y]);
        std::copy(newPng[midY + y] + midX * 4, newPng[midY + y] + rightX * 4, bottomRight[y]);
    }

    if (exchangeType == "clockwise") {
        for (int y = 0; y < exchange_height / 2; ++y) {
            std::copy(bottomLeft[y], bottomLeft[y] + exchange_width / 2 * 4, newPng[leftY + y] + leftX * 4);
            std::copy(topLeft[y], topLeft[y] + exchange_width / 2 * 4, newPng[leftY + y] + midX * 4);
            std::copy(bottomRight[y], bottomRight[y] + exchange_width / 2 * 4, newPng[midY + y] + leftX * 4);
            std::copy(topRight[y], topRight[y] + exchange_width / 2 * 4, newPng[midY + y] + midX * 4);
        }
    } else if (exchangeType == "counterclockwise") {
        for (int y = 0; y < exchange_height / 2; ++y) {
            std::copy(topRight[y], topRight[y] + exchange_width / 2 * 4, newPng[leftY + y] + leftX * 4);
            std::copy(bottomRight[y], bottomRight[y] + exchange_width / 2 * 4, newPng[leftY + y] + midX * 4);
            std::copy(topLeft[y], topLeft[y] + exchange_width / 2 * 4, newPng[midY + y] + leftX * 4);
            std::copy(bottomLeft[y], bottomLeft[y] + exchange_width / 2 * 4, newPng[midY + y] + midX * 4);
        }
    } else if (exchangeType == "diagonals") {
        for (int y = 0; y < exchange_height / 2; ++y) {
            std::copy(bottomRight[y], bottomRight[y] + exchange_width / 2 * 4, newPng[leftY + y] + leftX * 4);
            std::copy(bottomLeft[y], bottomLeft[y] + exchange_width / 2 * 4, newPng[leftY + y] + midX * 4);
            std::copy(topRight[y], topRight[y] + exchange_width / 2 * 4, newPng[midY + y] + leftX * 4);
            std::copy(topLeft[y], topLeft[y] + exchange_width / 2 * 4, newPng[midY + y] + midX * 4);
        }
    }
}

Rgb findMostFrequentColor(int &height, int &width, png_bytep* &newPng) {
    std::map<int, std::map<int, std::map<int, int>>> colorCount;

    for (int y = 0; y < height; ++y) {
        png_bytep row = newPng[y];
        for (int x = 0; x < width; ++x) {
            png_bytep px = &(row[x * 3]);
            int r = px[0];
            int g = px[1];
            int b = px[2];
            colorCount[r][g][b]++;
        }
    }
    int maxCount = 0;
    Rgb mostFrequentColor;
    for (const auto &r : colorCount) {
        for (const auto &g : r.second) {
            for (const auto &b : g.second) {
                if (b.second > maxCount) {
                    mostFrequentColor.R = r.first;
                    mostFrequentColor.G = g.first;
                    mostFrequentColor.B = b.first;
                    maxCount = b.second;
                }
            }
        }
    }
    return mostFrequentColor;
}

void processFreqCommand(int &height, int &width,png_bytep* & newPng, int argc, char* argv[]){
	int opt;
	int keyIndex;
	std::string color = "";
	int arrayColor[3];
	while(true){
		opt = getopt_long(argc, argv, "", freqKeys, &keyIndex);
		if(opt == -1){
			opt = 0;			
			break;
		}
		switch(opt){
			case 'l':
				if(std::regex_match(optarg, std::regex
				("(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9]).(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9]).(25[0-5]|2[0-4][0-9]|[01]?[0-9]?[0-9])"))){
					color = optarg;
					getRgb(color, arrayColor);
				}
				break;
		}
	}
	if(color == ""){
		std::cout << "Invalid data\n";
		exit(41);
	}
	Rgb freqColor = findMostFrequentColor(height, width, newPng);
	for(int i = 0; i < width; i++){
		for(int k = 0; k < height; k++){
			if(newPng[k][4*i] == freqColor.R &&
			newPng[k][4*i + 1] == freqColor.G &&
			newPng[k][4*i + 2] == freqColor.B){
				newPng[k][4*i] = arrayColor[0];
				newPng[k][4*i + 1] = arrayColor[1];
				newPng[k][4*i + 2] = arrayColor[2];
			}
		}
	}
	
}

bool processCommand(png_bytep* &newPng, int &height, int &width,  png_byte &colorType, png_byte &bitDepth, int argc, char* argv[], std::string &outputName, std::string &inputName){
    int opt;
	int keyIndex;
	opterr = 0;
	bool validFileName = false;
	bool square = false;
	bool exchange = false;
	bool freq_color = false;
	bool printInfo = false;
	int count = 0;
	char** argvCopy = new char*[argc];
    for (int i = 0; i < argc; ++i) {
        size_t len = strlen(argv[i]) + 1;
        argvCopy[i] = new char[len];
        strncpy(argvCopy[i], argv[i], len);
    }
	if(argc == 1) outputHelp();
	while(true){
		opt = getopt_long(argc, argvCopy, "ho:i", selectAction, &keyIndex);
		if(opt == -1){
			opt = 0;
			break;
		}
		switch(opt){
			case 'h':
				outputHelp();
				break;
			case 'o':
				if(checkValidName(optarg)) outputName = optarg;
				break;
			case 'i':
				if(checkValidName(optarg)){
                    inputName = optarg;
					if(read_png_file(inputName, newPng, height, width, colorType, bitDepth)) validFileName = true;
				}
				break;
			case 'd':
				printInfo = true;
				break;
			case 'r':
				square = true;
				count++;
				break;
			case 'x':
				exchange = true;
				count++;
				break;
			case 'c':
				freq_color = true;
				count++;
				break;
		}
	}
	if(printInfo){
		if(validFileName) outputInfo(height, width);
		else{
			std::cout << "Error: the input file name is invalid or the input file is corrupted" << std::endl;
			exit(41);
		}
	}
	optind = 1;
	if(count > 1){
		std::cout << "Error: too many arguments" << std::endl;
		exit(41);
	}
	else{
		if(validFileName&&square) processSquareCommand(height, width, newPng, argc, argv);
		else if(validFileName&&exchange) processExchangeCommand(height, width, newPng, argc, argv);
		else if(validFileName&&freq_color) processFreqCommand(height, width, newPng, argc, argv);
		else if(square || exchange || freq_color){
			std::cout << "Invalid input file name" << std::endl;
			exit(41);
		}
	}
	return validFileName;
};

int main(int argc, char* argv[]){
    png_bytep* newPng = nullptr;
    int height;
    int width;
    png_byte colorType;
    png_byte bitDepth;
	char filename[] = "fd.png";
	std::string outputName = "out.png";
	std::string inputName = "";
	std::cout << CW << std::endl;
	if(findUnknownKey(argc, argv)){
		exit(41);
		return 0;
	}
	if(processCommand(newPng, height, width, colorType, bitDepth, argc, argv, outputName, inputName)){
    	if(outputName == inputName){
			std::cout << "Error: the names of the input and output files are the same" << std::endl;
			exit(41);
		} else write_png_file(outputName, newPng, height, width);
	  return 0;
  }
}