#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
using namespace std;

// ═══════════════════════════════════════════════════════
// PIXEL
// ═══════════════════════════════════════════════════════
class Pixel {
private:
    int r, g, b;

public:
    Pixel() { r = g = b = 0; }//default constr

    Pixel(int r, int g, int b) {//parametrized through *this ptr
        this->r = clamp(r);
        this->g = clamp(g);
        this->b = clamp(b);
    }

    int getR() const { //getters
    return r; }
    int getG() const { 
    return g; }
    int getB() const { 
    return b; }

    void setR(int v) {//setters
     r = clamp(v); 
     }
    void setG(int v) {
     g = clamp(v); 
     }
    void setB(int v) {
     b = clamp(v); 
     }

    static int clamp(int v) {//to clamp brightness between 0to 255
        if (v < 0)   return 0;
        if (v > 255) return 255;
        return v;
    }

    int getBrightness() const { return (r + g + b) / 3; }

 char toAsciiChar() const {//as 9 charatcers were given here so we got a range of 28 characters for ewach of the asccii
    int br = getBrightness();
    if (br < 28)  return ' ';
    if (br < 56)  return '.';
    if (br < 84)  return ':';
    if (br < 112) return '-';
    if (br < 140) return '=';
    if (br < 168) return '+';
    if (br < 196) return '*';
    if (br < 224) return '#';
    return '@'; // 224 to 255
}

    Pixel operator+(const Pixel& p) const {//operator overloading to add 
        return Pixel(r + p.r, g + p.g, b + p.b);
    }

    friend ostream& operator<<(ostream& out, const Pixel& px) {//operator overaloding to cout
        out << "(" << px.r << ", " << px.g << ", " << px.b << ")";
        return out;
    }
};

// SAVEABLE  (interface)

class Saveable {
public:
    virtual bool saveToFile(const string& path) = 0;
    virtual ~Saveable() {}
};

// IMAGE
class FilterSession; // forward declaration for friend

class Image : public Saveable {//inheritance
private:
    Pixel** pixels;
    int width;
    int height;

public:
    friend class FilterSession;

    Image(int w, int h) {    //parameterized constr
        width  = w;
        height = h;
        pixels = new Pixel*[height];
        for (int i = 0; i < height; i++)
            pixels[i] = new Pixel[width];
    }

    Image(const Image& src) { //Deep cpy constr
        width  = src.width;
        height = src.height;
        pixels = new Pixel*[height];
        for (int i = 0; i < height; i++) {
            pixels[i] = new Pixel[width];
            for (int j = 0; j < width; j++)
                pixels[i][j] = src.pixels[i][j];
        }
    }

    // Destructor
    ~Image() {
        for (int i = 0; i < height; i++)
            delete[] pixels[i];
        delete[] pixels;
    }

    Pixel& at(int r, int c) { return pixels[r][c]; }

    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    void displayAscii() const {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++)
                cout << pixels[i][j].toAsciiChar();
            cout << "\n";
        }
    }

    void generateTestPattern() {
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++)
                pixels[i][j] = Pixel(i * 20, j * 20, 100);
    }

 
    bool loadFromFile(const string& path) {//handeled by image filemanager
        int w, h, channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 3);
        if (!data) return false;
        // resize
        for (int i = 0; i < height; i++) delete[] pixels[i];
        delete[] pixels;
        width  = w;
        height = h;
        pixels = new Pixel*[height];
        for (int i = 0; i < height; i++) {
            pixels[i] = new Pixel[width];
            for (int j = 0; j < width; j++) {
                int idx = (i * w + j) * 3;
                pixels[i][j] = Pixel(data[idx], data[idx+1], data[idx+2]);
            }
        }
        stbi_image_free(data);
        return true;
    }

    bool saveToFile(const string& path) override {
        unsigned char* data = new unsigned char[width * height * 3];
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++) {
                int idx = (i * width + j) * 3;
                data[idx]   = pixels[i][j].getR();
                data[idx+1] = pixels[i][j].getG();
                data[idx+2] = pixels[i][j].getB();
            }
        int result = stbi_write_png(path.c_str(), width, height, 3, data, width * 3);
        delete[] data;
        return result != 0;
    }
};

// FILTER  (abstract)

class Filter {
protected:
    int    filterId;
    string filterName;
    string category;
    bool   isEnabled;

public:
    Filter(int id, string name, string cat) {
        filterId   = id;
        filterName = name;
        category   = cat;
        isEnabled  = true;
    }

    virtual ~Filter() {}

    virtual void   apply(Image& image) = 0;
    virtual string getName() const { 
    return filterName; 
    }

    int    getId()         const {
    return filterId; 
    }
    string getCategory()   const {
    return category;
    }
    bool   getIsEnabled()  const { 
    return isEnabled;
    }
    void   setIsEnabled(bool v)  { isEnabled = v; }
};


// TEN FILTERS

class GrayscaleFilter : public Filter {
public:
    GrayscaleFilter() : Filter(1, "Grayscale", "Pixel Transform") {}

    void apply(Image& image) override {
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p  = image.at(i, j);
                int gray  = p.getBrightness();
                p = Pixel(gray, gray, gray);
            }
    }
    string getName() const override { return "Grayscale"; }
};

class Invert : public Filter {
public:
    Invert() : Filter(2, "Invert", "Pixel Transform") {}

    void apply(Image& image) override {
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p = image.at(i, j);
                p = Pixel(255 - p.getR(), 255 - p.getG(), 255 - p.getB());
            }
    }
    string getName() const override { 
    return "Invert"; 
    }
};

class BrightnessFilter : public Filter {
private:
    int amount;
public:
    BrightnessFilter(int v) : Filter(3, "Brightness Adjust", "Pixel Transform") {
        amount = v;
    }

    void apply(Image& image) override {
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p = image.at(i, j);
                p = Pixel(p.getR() + amount, p.getG() + amount, p.getB() + amount);
            }
    }

    void setAmount(int v) { amount = v; }
    string getName() const override { 
    return "Brightness Adjust"; 
    }
};

class ConstrastFilter : public Filter {
public:
    ConstrastFilter() : Filter(4, "Contrast Stretch", "Pixel Transform") {}

    void apply(Image& image) override {
        int minVal = 255, maxVal = 0;
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                int br = image.at(i, j).getBrightness();
                if (br < minVal) minVal = br;
                if (br > maxVal) maxVal = br;
            }

        if (maxVal == minVal) return; 

        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p = image.at(i, j);
                int r = (p.getR() - minVal) * 255 / (maxVal - minVal);
                int g = (p.getG() - minVal) * 255 / (maxVal - minVal);
                int b = (p.getB() - minVal) * 255 / (maxVal - minVal);
                p = Pixel(r, g, b);
            }
    }
    string getName() const override {
    return "Contrast Stretch"; 
    }
};

class RedChannelFilter : public Filter {
public:
    RedChannelFilter() : Filter(5, "Red Channel Only", "Pixel Transform") {}

    void apply(Image& image) override {
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p = image.at(i, j);
                p = Pixel(p.getR(), 0, 0);
            }
    }
    string getName() const override { 
    return "Red Channel Only"; 
    }
};

class GreenChannelFilter : public Filter {
public:
    GreenChannelFilter() : Filter(6, "Green Channel Only", "Pixel Transform") {}

    void apply(Image& image) override {
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p = image.at(i, j);
                p = Pixel(0, p.getG(), 0);
            }
    }
    string getName() const override { return "Green Channel Only"; }
};

class BlueChannelFilter : public Filter {
public:
    BlueChannelFilter() : Filter(7, "Blue Channel Only", "Pixel Transform") {}

    void apply(Image& image) override {
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel& p = image.at(i, j);
                p = Pixel(0, 0, p.getB());
            }
    }
    string getName() const override { return "Blue Chasnnel Only"; }
};

class BoxBlurFilter : public Filter {
public:
    BoxBlurFilter() : Filter(8, "Box Blur (3x3)", "Spatial Filter") {}

    void apply(Image& image) override {
        Image copy = image; 
        for (int i = 0; i < image.getHeight(); i++) {
            for (int j = 0; j < image.getWidth(); j++) {
                int sumR = 0, sumG = 0, sumB = 0, count = 0;
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < image.getHeight() &&
                            nj >= 0 && nj < image.getWidth()) {
                            Pixel p = copy.at(ni, nj);
                            sumR += p.getR();
                            sumG += p.getG();
                            sumB += p.getB();
                            count++;
                        }
                    }
                }
                image.at(i, j) = Pixel(sumR / count, sumG / count, sumB / count);
            }
        }
    }
    string getName() const override { return "Box Blur (3x3)"; }
};

class FlipHorizontalFilter : public Filter {
public:
    FlipHorizontalFilter() : Filter(9, "Flip Horizontal", "Geometric") {}

    void apply(Image& image) override {
        int w = image.getWidth();
        for (int i = 0; i < image.getHeight(); i++)
            for (int j = 0; j < w / 2; j++) {
                Pixel temp         = image.at(i, j);
                image.at(i, j)     = image.at(i, w - 1 - j);
                image.at(i, w-1-j) = temp;
            }
    }
    string getName() const override { return "Flip Horizontal"; }
};

class FlipVerticalFilter : public Filter {
public:
    FlipVerticalFilter() : Filter(10, "Flip Vertical", "Geometric") {}

    void apply(Image& image) override {
        int h = image.getHeight();
        for (int i = 0; i < h / 2; i++)
            for (int j = 0; j < image.getWidth(); j++) {
                Pixel temp           = image.at(i, j);
                image.at(i, j)       = image.at(h - 1 - i, j);
                image.at(h - 1 - i, j) = temp;
            }
    }
    string getName() const override { return "Flip Vertical"; }
};


// IMAGE FILE MANAGER

class ImageFileManager {
private:
    string filePath;

public:
    ImageFileManager() { filePath = ""; }

    Image* loadImage(const string& path) {//DMA
        int w, h, channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 3);
        if (!data) {
            cout << "ERROR: Could not load: " << path << "\n";
            return nullptr;
        }
        Image* img = new Image(w, h);
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++) {
                int idx = (i * w + j) * 3;
                img->at(i, j) = Pixel(data[idx], data[idx+1], data[idx+2]);
            }
        stbi_image_free(data);
        filePath = path;
        cout << "Image loaded: " << w << " x " << h << " pixels\n";
        return img;
    }

    bool saveAsPng(const string& path, Image& image) {
        int w = image.getWidth(), h = image.getHeight();
        unsigned char* data = new unsigned char[w * h * 3];
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++) {
                int idx     = (i * w + j) * 3;
                data[idx]   = image.at(i, j).getR();
                data[idx+1] = image.at(i, j).getG();
                data[idx+2] = image.at(i, j).getB();
            }
        int result = stbi_write_png(path.c_str(), w, h, 3, data, w * 3);
        delete[] data;
        if (result) { cout << "Saved PNG: " << path << "\n"; return true; }
        cout << "ERROR: Failed to save PNG.\n";
        return false;
    }

    bool saveAsJpg(const string& path, Image& image) {
        int w = image.getWidth(), h = image.getHeight();
        unsigned char* data = new unsigned char[w * h * 3];
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++) {
                int idx     = (i * w + j) * 3;
                data[idx]   = image.at(i, j).getR();
                data[idx+1] = image.at(i, j).getG();
                data[idx+2] = image.at(i, j).getB();
            }
        int result = stbi_write_jpg(path.c_str(), w, h, 3, data, 90);
        delete[] data;
        if (result) { cout << "Saved JPG: " << path << "\n"; return true; }
        cout << "ERROR: Failed to save JPG.\n";
        return false;
    }
};


// FILTER SESSION

class FilterSession {//Composition
private:
    Filter* pipeline[10];
    int     pipelineSize;
    Image*  orignal;  
    Image*  current;
    string  customerCnic;
    string  timestamp;
    string  outputFile;

public:
    FilterSession(string cnic) {
        customerCnic = cnic;
        pipelineSize = 0;
        orignal  = nullptr;
        current  = nullptr;
        for (int i = 0; i < 10; i++) pipeline[i] = nullptr;
    }

    ~FilterSession() {
        delete orignal;
        delete current;
    }

    bool loadImage(const string& path) {
        ImageFileManager fm;
        if (orignal) { delete orignal; orignal = nullptr; }
        if (current) { delete current; current = nullptr; }
        orignal = fm.loadImage(path);
        if (!orignal) return false;
        current = new Image(*orignal);
        return true;
    }

    void loadTestPattern(int w, int h) {
        if (orignal) { delete orignal; orignal = nullptr; }
        if (current) { delete current; current = nullptr; }
        orignal = new Image(w, h);
        orignal->generateTestPattern();
        current = new Image(*orignal);
    }

    FilterSession& addFilter(Filter* f) {
        if (pipelineSize < 10)
            pipeline[pipelineSize++] = f;
        return *this; // method chaining used here
    }

    void removeFilter(int idx) {
        if (idx < 0 || idx >= pipelineSize) return;
        for (int i = idx; i < pipelineSize - 1; i++)
            pipeline[i] = pipeline[i + 1];
        pipeline[--pipelineSize] = nullptr;
    }

    void clearPipeline() {
        for (int i = 0; i < pipelineSize; i++) pipeline[i] = nullptr;
        pipelineSize = 0;
    }

    void applyPipeline() {
        if (!orignal) return;
        delete current;
        current = new Image(*orignal);
        for (int i = 0; i < pipelineSize; i++) {
            cout << "Applying filter " << (i+1) << "/" << pipelineSize
                 << ": " << pipeline[i]->getName() << " ...\n";
            pipeline[i]->apply(*current);
            displayCurrentPreview();
        }
    }

    bool saveResult(const string& path) {
        if (!current) return false;
        ImageFileManager fm;
        outputFile = path;
        return fm.saveAsPng(path, *current);
    }

    void displayCurrentPreview() {
        if (!current) { cout << "(no image loaded)\n"; return; }
        cout << "=== Image Preview ("
             << current->getWidth() << " x " << current->getHeight() << ") ===\n";
        current->displayAscii();
    }

    void displayPipeline() {
        if (pipelineSize == 0) { cout << "(empty)"; return; }
        cout << "[ ";
        for (int i = 0; i < pipelineSize; i++) {
            cout << pipeline[i]->getName();
            if (i != pipelineSize - 1) cout << " -> ";
        }
        cout << " ]";
    }

    string getOutputFile()      { return outputFile; }
    string getTimestamp()       { return timestamp; }
    string getFiltersApplied() {
        string s = "";
        for (int i = 0; i < pipelineSize; i++) {
            s += pipeline[i]->getName();
            if (i != pipelineSize - 1) s += ">";
        }
        return s;
    }

    void   apply(Image& img) { applyPipeline(); }
    string getName()         { return "FilterSession"; }
};


// STRUCTS for file managers 

struct CustomerRecord {
    string cnic;
    string password;
    string fullName;
    string gender;
    string phone;
    string city;
    int    isBlocked;
};

struct CatalogRecord {
    int    id;
    string name;
    string category;
    int    isEnabled;
};


// BLOCKED CNIC MANAGER

class BlockedCnicManager {
private:
    string filePath;

public:
    BlockedCnicManager() { filePath = "blocked_cnics.txt"; }

    bool isCnicBlocked(const string& cnic) {
        ifstream file(filePath);
        if (!file.is_open()) return false;
        string line;
        while (getline(file, line))
            if (line == cnic) { file.close(); return true; }
        file.close();
        return false;
    }

    void addBlockedCnic(const string& cnic) {
        ofstream file(filePath, ios::app);
        if (file.is_open()) { file << cnic << "\n"; file.close(); }
    }

    void loadAll(string arr[], int& sz) {
        sz = 0;
        ifstream file(filePath);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line) && sz < 100)
            arr[sz++] = line;
        file.close();
    }
};

// CUSTOMER FILE MANAGER

class CustomerFileManager {
private:
    string filePath;

    CustomerRecord parseLine(const string& line) {
        CustomerRecord rec;
        string parts[7];
        int count = 0;
        string token = "";
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == '|') { parts[count++] = token; token = ""; }
            else token += line[i];
        }
        parts[count++] = token;
        rec.cnic      = parts[0];
        rec.password  = parts[1];
        rec.fullName  = parts[2];
        rec.gender    = parts[3];
        rec.phone     = parts[4];
        rec.city      = parts[5];
        rec.isBlocked = (parts[6] == "1") ? 1 : 0;
        return rec;
    }

    string recordToLine(const CustomerRecord& rec) {
        return rec.cnic + "|" + rec.password + "|" + rec.fullName + "|" +
               rec.gender + "|" + rec.phone + "|" + rec.city + "|" +
               (rec.isBlocked ? "1" : "0");
    }

public:
    CustomerFileManager() { filePath = "customers.txt"; }

    void loadAll(CustomerRecord arr[], int& sz) {
        sz = 0;
        ifstream file(filePath);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line) && sz < 100) {
            if (line.empty()) continue;
            arr[sz++] = parseLine(line);
        }
        file.close();
    }

    void saveAll(CustomerRecord arr[], int sz) {
        ofstream file(filePath);
        if (!file.is_open()) { cout << "ERROR: Cannot open customers.txt\n"; return; }
        for (int i = 0; i < sz; i++) file << recordToLine(arr[i]) << "\n";
        file.close();
    }

    int findByCnic(const string& cnic, CustomerRecord arr[], int sz) {
        for (int i = 0; i < sz; i++)
            if (arr[i].cnic == cnic) return i;
        return -1;
    }

    int findByName(const string& name, CustomerRecord arr[], int sz) {
        for (int i = 0; i < sz; i++)
            if (arr[i].fullName == name) return i;
        return -1;
    }

    void addCustomer(const CustomerRecord& rec) {
        ofstream file(filePath, ios::app);
        if (file.is_open()) { file << recordToLine(rec) << "\n"; file.close(); }
    }

    void block(const string& cnic) {
        CustomerRecord arr[100]; int sz = 0;
        loadAll(arr, sz);
        for (int i = 0; i < sz; i++)
            if (arr[i].cnic == cnic) { arr[i].isBlocked = 1; break; }
        saveAll(arr, sz);
    }

    void deleteRecord(const string& cnic) {
        CustomerRecord arr[100]; int sz = 0;
        loadAll(arr, sz);
        CustomerRecord newArr[100]; int newSz = 0;
        for (int i = 0; i < sz; i++)
            if (arr[i].cnic != cnic) newArr[newSz++] = arr[i];
        saveAll(newArr, newSz);
    }
};


// CATALOG FILE MANAGER

class CatalogFileManager {
private:
    string filePath;

    CatalogRecord parseLine(const string& line) {
        CatalogRecord rec;
        string parts[4];
        int count = 0; string token = "";
        for (int i = 0; i < (int)line.size(); i++) {
            if (line[i] == '|') { parts[count++] = token; token = ""; }
            else token += line[i];
        }
        parts[count++] = token;
        rec.id        = stoi(parts[0]);
        rec.name      = parts[1];
        rec.category  = parts[2];
        rec.isEnabled = stoi(parts[3]);
        return rec;
    }

    string recordToLine(const CatalogRecord& rec) {
        return to_string(rec.id) + "|" + rec.name + "|" +
               rec.category + "|" + to_string(rec.isEnabled);
    }

public:
    CatalogFileManager() { filePath = "catalog.txt"; }

    void seedDefaults(CatalogRecord arr[], int& sz) {
        sz = 10;
        arr[0] = {1,  "Grayscale",         "Pixel Transform", 1};
        arr[1] = {2,  "Invert",             "Pixel Transform", 1};
        arr[2] = {3,  "Brightness Adjust",  "Pixel Transform", 1};
        arr[3] = {4,  "Contrast Stretch",   "Pixel Transform", 1};
        arr[4] = {5,  "Red Channel Only",   "Pixel Transform", 1};
        arr[5] = {6,  "Green Channel Only", "Pixel Transform", 1};
        arr[6] = {7,  "Blue Channel Only",  "Pixel Transform", 1};
        arr[7] = {8,  "Box Blur (3x3)",     "Spatial Filter",  1};
        arr[8] = {9,  "Flip Horizontal",    "Geometric",       1};
        arr[9] = {10, "Flip Vertical",      "Geometric",       1};
    }

    void loadCatalog(CatalogRecord arr[], int& sz) {
        sz = 0;
        ifstream file(filePath);
        if (!file.is_open()) { seedDefaults(arr, sz); saveCatalog(arr, sz); return; }
        string line;
        while (getline(file, line) && sz < 10) {
            if (line.empty()) continue;
            arr[sz++] = parseLine(line);
        }
        file.close();
    }

    void saveCatalog(CatalogRecord arr[], int sz) {
        ofstream file(filePath);
        if (!file.is_open()) return;
        for (int i = 0; i < sz; i++) file << recordToLine(arr[i]) << "\n";
        file.close();
    }

    void toggleFilter(int id) {
        CatalogRecord arr[10]; int sz = 0;
        loadCatalog(arr, sz);
        for (int i = 0; i < sz; i++)
            if (arr[i].id == id) { arr[i].isEnabled = arr[i].isEnabled ? 0 : 1; break; }
        saveCatalog(arr, sz);
    }
};


// SESSION FILE MANAGER

class SessionFileManager {
private:
    string filePath;

public:
    SessionFileManager() { filePath = "sessions.txt"; }

    void appendSession(const string& cnic, const string& timestamp,
                       const string& filtersApplied, const string& outputFile) {
        ofstream file(filePath, ios::app);
        if (file.is_open()) {
            file << cnic << "|" << timestamp << "|"
                 << filtersApplied << "|" << outputFile << "\n";
            file.close();
        }
    }

    void loadByCnic(const string& cnic, string arr[], int& sz) {
        sz = 0;
        ifstream file(filePath);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line) && sz < 50)
            if (line.size() >= cnic.size() && line.substr(0, cnic.size()) == cnic)
                arr[sz++] = line;
        file.close();
    }

    void loadAll(string arr[], int& sz) {
        sz = 0;
        ifstream file(filePath);
        if (!file.is_open()) return;
        string line;
        while (getline(file, line) && sz < 200)
            if (!line.empty()) arr[sz++] = line;
        file.close();
    }
};


// USER  (abstract)

class User {
protected:
    string cnic;
    string password;
    string fullName;
    string gender;
    string phone;
    string city;

public:
    User(string c, string p, string name, string g, string ph, string ct) {
        cnic     = c;
        password = p;
        fullName = name;
        gender   = g;
        phone    = ph;
        city     = ct;
        cout << "[User created: " << fullName << "]\n";
    }

    virtual ~User() {
        cout << "[User destroyed: " << fullName << "]\n";
    }

    bool login(const string& enteredPass) { return enteredPass == password; }

    void logout() { cout << "Goodbye, " << fullName << "!\n"; }

    string getCnic()     const { return cnic; }
    string getFullName() const { return fullName; }
    string getPassword() const { return password; }

    bool validatePassword(const string& p) {
        if ((int)p.size() != 9) return false;
        bool hasUpper = false, hasDigit = false;
        for (int i = 0; i < 9; i++) {
            if (p[i] >= 'A' && p[i] <= 'Z') hasUpper = true;
            if (p[i] >= '0' && p[i] <= '9') hasDigit = true;
        }
        return hasUpper && hasDigit;
    }

    bool validateCnic(const string& c) {
        if ((int)c.size() != 13) return false;
        for (int i = 0; i < 13; i++)
            if (c[i] < '0' || c[i] > '9') return false;
        return true;
    }

    virtual void showMenu() = 0; // pure virtual as User is abstract
};


// ADMIN

class Admin : public User {
private:
    string adminId;
    CustomerFileManager cfm;
    CatalogFileManager  catfm;
    SessionFileManager  sfm;
    BlockedCnicManager  bcm;

public:
    Admin() : User("0000000000000", "Admin123", "Administrator", "M", "0000", "HQ") {
        adminId = "ADMIN01";
        cout << "[Admin constructed]\n";
    }

    ~Admin() { cout << "[Admin destroyed]\n"; }

    void showMenu() override {
        int choice = 0;
        do {
            cout << "\n╔══════════════════════════════════════════╗\n";
            cout <<   "║    ADMIN PANEL : Image Filter Studio     ║\n";
            cout <<   "╠══════════════════════════════════════════╣\n";
            cout <<   "║  1. Manage Filter Catalog                ║\n";
            cout <<   "║  2. Manage Customers                     ║\n";
            cout <<   "║  3. View All Sessions                    ║\n";
            cout <<   "║  4. Logout                               ║\n";
            cout <<   "╚══════════════════════════════════════════╝\n";
            cout << "Your choice: ";
            cin >> choice; cin.ignore();
            if      (choice == 1) manageFilterCatalog();
            else if (choice == 2) manageCustomers();
            else if (choice == 3) viewAllSessions();
        } while (choice != 4);
        logout();
    }

    void manageFilterCatalog() {
        CatalogRecord arr[10]; int sz = 0;
        catfm.loadCatalog(arr, sz);
        cout << "\n=== Filter Catalog ===\n";
        for (int i = 0; i < sz; i++)
            cout << arr[i].id << ". " << arr[i].name
                 << " [" << arr[i].category << "] "
                 << (arr[i].isEnabled ? "ENABLED" : "DISABLED") << "\n";
        cout << "Enter filter ID to toggle (0 = back): ";
        int id; cin >> id; cin.ignore();
        if (id != 0) { toggleFilter(id); cout << "Filter toggled.\n"; }
    }

    void toggleFilter(int id) { catfm.toggleFilter(id); }

    void manageCustomers() {
        cout << "\n=== Manage Customers ===\n";
        cout << "1. View all  2. Search  3. Block  4. Delete  0. Back\nChoice: ";
        int choice; cin >> choice; cin.ignore();
        if      (choice == 1) viewCustomerTable();
        else if (choice == 2) searchCustomer("");
        else if (choice == 3) {
            cout << "CNIC to block: "; string c; getline(cin, c); blockCustomer(c);
        }
        else if (choice == 4) {
            cout << "CNIC to delete: "; string c; getline(cin, c); deleteCustomer(c);
        }
    }

    void viewCustomerTable() {
        CustomerRecord arr[100]; int sz = 0;
        cfm.loadAll(arr, sz);
        cout << "\nCNIC          | Name                | Blocked\n";
        cout << "------------------------------------------------\n";
        for (int i = 0; i < sz; i++)
            cout << arr[i].cnic << " | " << arr[i].fullName
                 << " | " << (arr[i].isBlocked ? "YES" : "NO") << "\n";
        if (sz == 0) cout << "(no customers)\n";
    }

    void searchCustomer(const string& q) {
        string query = q;
        if (query.empty()) { cout << "Search (CNIC or name): "; getline(cin, query); }
        CustomerRecord arr[100]; int sz = 0;
        cfm.loadAll(arr, sz);
        bool found = false;
        for (int i = 0; i < sz; i++)
            if (arr[i].cnic == query || arr[i].fullName == query) {
                cout << "Found: " << arr[i].fullName << " | " << arr[i].cnic
                     << " | Blocked: " << arr[i].isBlocked << "\n";
                found = true;
            }
        if (!found) cout << "No customer found.\n";
    }

    void blockCustomer(const string& cnic) {
        cfm.block(cnic);
        bcm.addBlockedCnic(cnic);
        cout << "Customer " << cnic << " blocked.\n";
    }

    void deleteCustomer(const string& cnic) {
        cfm.deleteRecord(cnic);
        cout << "Customer " << cnic << " deleted.\n";
    }

    void viewAllSessions() {
        string arr[200]; int sz = 0;
        sfm.loadAll(arr, sz);
        cout << "\n=== All Sessions ===\n";
        for (int i = 0; i < sz; i++) cout << arr[i] << "\n";
        if (sz == 0) cout << "(no sessions yet)\n";
    }
};

// CUSTOMER

class Customer : public User {
private:
    bool           isBlocked;
    int            sessionCount;
    FilterSession* session;

    CustomerFileManager cfm;
    CatalogFileManager  catfm;
    SessionFileManager  sfm;

    Filter* catalog[10];
    int     catalogSize;

    void buildCatalog() {
        catalogSize = 0;
        catalog[catalogSize++] = new GrayscaleFilter();
        catalog[catalogSize++] = new Invert();
        catalog[catalogSize++] = new BrightnessFilter(0);
        catalog[catalogSize++] = new ConstrastFilter();
        catalog[catalogSize++] = new RedChannelFilter();
        catalog[catalogSize++] = new GreenChannelFilter();
        catalog[catalogSize++] = new BlueChannelFilter();
        catalog[catalogSize++] = new BoxBlurFilter();
        catalog[catalogSize++] = new FlipHorizontalFilter();
        catalog[catalogSize++] = new FlipVerticalFilter();
    }

    void freeCatalog() {
        for (int i = 0; i < catalogSize; i++) { delete catalog[i]; catalog[i] = nullptr; }
        catalogSize = 0;
    }

    string makeTimestamp() {
        time_t now = time(0);
        tm* t = localtime(&now);
        char buf[32];
        sprintf(buf, "%04d%02d%02d_%02d%02d%02d",
            t->tm_year+1900, t->tm_mon+1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
        return string(buf);
    }

public:
    Customer(string c, string p, string name, string g, string ph, string ct)
        : User(c, p, name, g, ph, ct) {
        isBlocked    = false;
        sessionCount = 0;
        session      = nullptr;
        buildCatalog();
        cout << "[Customer constructed: " << fullName << "]\n";
    }

    ~Customer() {
        if (session) { delete session; session = nullptr; }
        freeCatalog();
        cout << "[Customer destroyed: " << fullName << "]\n";
    }

    bool getIsBlocked()      const { return isBlocked; }
    void setIsBlocked(bool v)      { isBlocked = v; }
    void incrementSessions()       { sessionCount++; }

    void showMenu() override {
        int choice = 0;
        do {
            cout << "\n╔══════════════════════════════════════════╗\n";
            cout <<   "║  Welcome, " << fullName << "\n";
            cout <<   "║  Sessions completed: " << sessionCount << "\n";
            cout <<   "╠══════════════════════════════════════════╣\n";
            cout <<   "║  1. Browse Filter Catalog                ║\n";
            cout <<   "║  2. Load Image                           ║\n";
            cout <<   "║  3. Build Filter Pipeline                ║\n";
            cout <<   "║  4. Apply Pipeline & Save Result         ║\n";
            cout <<   "║  5. View My Session History              ║\n";
            cout <<   "║  6. Logout                               ║\n";
            cout <<   "╚══════════════════════════════════════════╝\n";
            cout << "Your choice: ";
            cin >> choice; cin.ignore();
            if      (choice == 1) browseFilterCatalog();
            else if (choice == 2) loadImageFromUser();
            else if (choice == 3) buildPipeline();
            else if (choice == 4) applyAndSave();
            else if (choice == 5) viewSessionHistory();
        } while (choice != 6);
        logout();
    }

    void browseFilterCatalog() {
        CatalogRecord arr[10]; int sz = 0;
        catfm.loadCatalog(arr, sz);
        cout << "\n=== Available Filters ===\n";
        for (int i = 0; i < sz; i++)
            cout << arr[i].id << ". " << arr[i].name
                 << " [" << arr[i].category << "] "
                 << (arr[i].isEnabled ? "Enabled" : "DISABLED") << "\n";
    }

    void loadImageFromUser() {
        if (session) { delete session; session = nullptr; }
        session = new FilterSession(cnic);
        cout << "\n=== Load Image ===\n1. Load from file\n2. Test pattern\nChoice: ";
        int ch; cin >> ch; cin.ignore();
        if (ch == 1) {
            cout << "Enter image path: ";
            string path; getline(cin, path);
            if (!session->loadImage(path)) {
                cout << "Failed to load image.\n";
                delete session; session = nullptr; return;
            }
        } else {
            session->loadTestPattern(40, 20);
        }
        cout << "\n=== ASCII Preview ===\n";
        session->displayCurrentPreview();
    }

    void loadImage(const string& path) {
        if (session) { delete session; session = nullptr; }
        session = new FilterSession(cnic);
        session->loadImage(path);
    }

    void buildPipeline() {
        if (!session) { cout << "Load an image first.\n"; return; }
        session->clearPipeline();
        CatalogRecord arr[10]; int sz = 0;
        catfm.loadCatalog(arr, sz);
        int id = -1;
        do {
            browseFilterCatalog();
            cout << "\nCurrent pipeline: "; session->displayPipeline();
            cout << "\nAdd filter ID (0 to finish): ";
            cin >> id; cin.ignore();
            if (id == 0) break;
            bool enabled = false;
            for (int i = 0; i < sz; i++)
                if (arr[i].id == id && arr[i].isEnabled) { enabled = true; break; }
            if (!enabled) { cout << "Filter disabled or invalid.\n"; continue; }
            Filter* chosen = nullptr;
            for (int i = 0; i < catalogSize; i++)
                if (catalog[i]->getId() == id) { chosen = catalog[i]; break; }
            if (!chosen) continue;
            if (id == 3) {
                cout << "Brightness amount (-100 to +100): ";
                int amt; cin >> amt; cin.ignore();
                static_cast<BrightnessFilter*>(chosen)->setAmount(amt);
            }
            session->addFilter(chosen);
            cout << "Added: " << chosen->getName() << "\n";
        } while (id != 0);
        cout << "Pipeline finalised.\n";
    }

    void applyAndSave() {
        if (!session) { cout << "Load an image and build pipeline first.\n"; return; }
        cout << "\n=== Applying Pipeline ===\n";
        session->applyPipeline();
        cout << "\nSave result? (y/n): ";
        char ans; cin >> ans; cin.ignore();
        if (ans == 'y' || ans == 'Y') {
            string ts      = makeTimestamp();
            string outFile = cnic + "_" + ts + ".png";
            bool ok = session->saveResult(outFile);
            if (ok) {
                sfm.appendSession(cnic, ts, session->getFiltersApplied(), outFile);
                incrementSessions();
                cout << "Session recorded in sessions.txt.\n";
                cout << "Open " << outFile << " in any image viewer.\n";
            }
        }
    }

    void viewSessionHistory() {
        string arr[50]; int sz = 0;
        sfm.loadByCnic(cnic, arr, sz);
        cout << "\n=== Your Session History ===\n";
        for (int i = 0; i < sz; i++) cout << arr[i] << "\n";
        if (sz == 0) cout << "(no sessions yet)\n";
    }

    void registerSelf() {
        CustomerRecord rec;
        rec.cnic      = cnic;
        rec.password  = password;
        rec.fullName  = fullName;
        rec.gender    = gender;
        rec.phone     = phone;
        rec.city      = city;
        rec.isBlocked = 0;
        cfm.addCustomer(rec);
        cout << "Registration successful!\n";
    }
};

// MAIN

int main() {
    cout << "[Program started:>]\n";

    CustomerFileManager cfm;
    BlockedCnicManager  bcm;
    int choice = 0;

    do {
        cout << "\n╔══════════════════════════════════════════╗\n";
        cout <<   "║         IMAGE FILTER STUDIO              ║\n";
        cout <<   "╠══════════════════════════════════════════╣\n";
        cout <<   "║  1. Admin Login                          ║\n";
        cout <<   "║  2. Customer Login                       ║\n";
        cout <<   "║  3. New Customer? Register here          ║\n";
        cout <<   "║  4. Exit                                 ║\n";
        cout <<   "╚══════════════════════════════════════════╝\n";
        cout << "Your choice: ";
        cin >> choice; cin.ignore();

        // ADMIN
        if (choice == 1) {
            cout << "Admin password: ";
            string pass; getline(cin, pass);
            if (pass == "Admin123") { Admin admin; admin.showMenu(); }
            else cout << "Wrong password.\n";
        }

        // CUSTOMER LOGIN 
        else if (choice == 2) {
            cout << "Enter CNIC: ";
            string cnic; getline(cin, cnic);
            int attempts = 0; bool loggedIn = false;
            while (attempts < 3 && !loggedIn) {
                cout << "Enter password: ";
                string pass; getline(cin, pass);
                CustomerRecord arr[100]; int sz = 0;
                cfm.loadAll(arr, sz);
                int idx = cfm.findByCnic(cnic, arr, sz);
                if (idx == -1)              { cout << "CNIC not found.\n"; break; }
                if (arr[idx].isBlocked)     { cout << "Account is blocked.\n"; break; }
                if (arr[idx].password == pass) {
                    Customer* cust = new Customer(arr[idx].cnic, arr[idx].password,
                        arr[idx].fullName, arr[idx].gender, arr[idx].phone, arr[idx].city);
                    cust->showMenu();
                    delete cust;
                    loggedIn = true;
                } else {
                    attempts++;
                    cout << "Wrong password. Attempts left: " << (3 - attempts) << "\n";
                }
            }
            if (!loggedIn && attempts == 3)
                cout << "Too many failed attempts. Returning to main menu.\n";
        }

        //REGISTER 
        else if (choice == 3) {
            cout << "\n=== New Customer Registration ===\n";
            string cnic, pass, confirm, name, gender, phone, city;
            bool valid = true;

            // CNIC validation
            while (true) {
                cout << "Enter CNIC (13 digits): ";
                getline(cin, cnic);
                if ((int)cnic.size() != 13) { cout << "Must be exactly 13 digits.\n"; continue; }
                bool allDigits = true;
                for (int i = 0; i < 13; i++)
                    if (cnic[i] < '0' || cnic[i] > '9') { allDigits = false; break; }
                if (!allDigits) { cout << "Digits only.\n"; continue; }
                if (bcm.isCnicBlocked(cnic)) { cout << "This CNIC is banned.\n"; valid = false; break; }
                CustomerRecord arr[100]; int sz = 0;
                cfm.loadAll(arr, sz);
                if (cfm.findByCnic(cnic, arr, sz) != -1) { cout << "CNIC already registered.\n"; valid = false; break; }
                break;
            }
            if (!valid) continue;

            // Password validation
            while (true) {
                cout << "Password (9 chars, 1 uppercase, 1 digit): ";
                getline(cin, pass);
                if ((int)pass.size() != 9) { cout << "Must be 9 characters.\n"; continue; }
                bool hasUpper = false, hasDigit = false;
                for (int i = 0; i < 9; i++) {
                    if (pass[i] >= 'A' && pass[i] <= 'Z') hasUpper = true;
                    if (pass[i] >= '0' && pass[i] <= '9') hasDigit = true;
                }
                if (!hasUpper || !hasDigit) { cout << "Need 1 uppercase and 1 digit.\n"; continue; }
                break;
            }
            cout << "Confirm password: "; getline(cin, confirm);
            if (confirm != pass) { cout << "Passwords do not match.\n"; continue; }

            cout << "Full name: ";          getline(cin, name);
            cout << "Gender (M/F/Other): "; getline(cin, gender);
            cout << "Phone: ";              getline(cin, phone);
            cout << "City: ";               getline(cin, city);

            Customer* newCust = new Customer(cnic, pass, name, gender, phone, city);
            newCust->registerSelf();
            delete newCust;
        }

    } while (choice != 4);

    cout << "[Program ended;)]\n";
    retur
