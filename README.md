# OOP_Project_Image-Filter-Studio
Console-based C++ Image Filter Studio — loads real JPG/PNG images, applies a polymorphic filter pipeline, and saves the result. Built to demonstrate core OOP concepts (inheritance, polymorphism, encapsulation, operator overloading, file I/O).
# Image Filter Studio

A console-based C++ application that loads **real JPG/PNG images**, decodes them into a pixel grid, applies a customizable pipeline of image filters, and writes the processed result back to disk as a viewable image.

Built as a semester project for CS1004 — Object Oriented Programming. The goal was to demonstrate every major OOP concept from the course in a single cohesive system, rather than as isolated examples.

---

## Features

- **Real image I/O** — reads and writes actual JPG/PNG files using [stb_image](https://github.com/nothings/stb) (no simulated pixel data)
- **10 image filters** across three categories: Pixel Transform, Spatial Filter, and Geometric
- **Filter pipeline** — chain multiple filters together; each runs on the output of the previous one
- **ASCII preview** — see a text-art approximation of the image directly in the terminal after each filter step
- **Role-based access** — separate Admin and Customer accounts with different permissions
- **Persistent storage** — all user data, filter catalog state, and session history saved to plain `.txt` files
- **Account validation** — CNIC format checks, password rules, login lockout after failed attempts, and account blocking

---

## Filter Catalog

| ID | Filter             | Category         | Effect                                              |
|----|--------------------|------------------|------------------------------------------------------|
| 01 | Grayscale          | Pixel Transform  | Replaces R,G,B with their average                    |
| 02 | Invert             | Pixel Transform  | Produces a colour negative                           |
| 03 | Brightness Adjust  | Pixel Transform  | Adds a user-defined value to every channel           |
| 04 | Contrast Stretch   | Pixel Transform  | Rescales brightness to use the full 0–255 range      |
| 05 | Red Channel Only   | Pixel Transform  | Isolates the red channel                             |
| 06 | Green Channel Only | Pixel Transform  | Isolates the green channel                           |
| 07 | Blue Channel Only  | Pixel Transform  | Isolates the blue channel                            |
| 08 | Box Blur (3×3)     | Spatial Filter   | Averages each pixel with its 8 neighbours            |
| 09 | Flip Horizontal    | Geometric        | Mirrors the image left-to-right                      |
| 10 | Flip Vertical      | Geometric        | Mirrors the image top-to-bottom                      |

---

## OOP Concepts Demonstrated

- **Abstraction** — `Filter` and `User` are abstract base classes with pure virtual functions
- **Inheritance** — 10 filter classes extend `Filter`; `Admin` and `Customer` extend `User`; `Image` implements the `Saveable` interface
- **Polymorphism** — the filter pipeline calls `apply()` through base-class pointers, dispatching to the correct override at runtime
- **Encapsulation** — all pixel data is private, accessed only via getters/setters
- **Operator Overloading** — `operator+` (pixel blending) and `operator<<` (formatted output) on `Pixel`
- **Dynamic Memory Management** — `Pixel**` 2D grid allocated on the heap with a deep-copy constructor and proper destructor
- **Friend Classes** — `FilterSession` is a friend of `Image`, allowing direct access to the pixel grid
- **Method Chaining** — `addFilter()` returns a reference to the session, enabling fluent pipeline construction
- **Static Members** — `Pixel::clamp()` is a class-level utility used across all filters

---

## Tech Stack

- **Language:** C++ (no STL containers — raw arrays and pointers throughout, per project constraints)
- **Image I/O:** [stb_image.h](https://github.com/nothings/stb) / [stb_image_write.h](https://github.com/nothings/stb) (single-header, public domain)
- **Storage:** Plain delimited `.txt` files (no database)

---

## Getting Started

### Prerequisites

Download `stb_image.h` and `stb_image_write.h` from the [stb repository](https://github.com/nothings/stb) and place them in the project root alongside `main.cpp`.

### Build

```bash
g++ main.cpp -o studio
```

### Run

```bash
./studio
```

---

## Project Structure

```
.
├── main.cpp              # All classes and program logic
├── stb_image.h           # Image loading (download separately)
├── stb_image_write.h     # Image writing (download separately)
├── customers.txt         # Generated at runtime — customer accounts
├── catalog.txt           # Generated at runtime — filter enable/disable state
├── sessions.txt          # Generated at runtime — session history log
└── blocked_cnics.txt     # Generated at runtime — banned CNICs
```

---

## Data File Formats

**customers.txt**
```
CNIC|Password|FullName|Gender|Phone|City|IsBlocked
```

**catalog.txt**
```
FilterID|FilterName|Category|IsEnabled
```

**sessions.txt**
```
CNIC|Timestamp|FiltersApplied|OutputFile
```

---

## Usage Flow

1. Launch the program and either log in as Admin, log in as an existing Customer, or register a new account
2. As a Customer: load a real image (or generate a test pattern), build a filter pipeline from the enabled filters, apply it with a live ASCII preview after each step, and save the result as a PNG
3. As an Admin: enable/disable filters in the catalog, manage customer accounts, and review all completed sessions

---

## License

This project was developed for academic purposes as part of a university OOP course.
