#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_PNG_Image.H>
#include <iostream>
#include <fstream>
#include <FL/Fl_Value_Input.H>
#include <vector>

Fl_Window *window;
Fl_Menu_Bar *menu;
Fl_Box *box;
Fl_PNG_Image *image;

class Renderer
{
public:
    int width = 1920;
    int height = 1080;

    CameraEditer()
    {
        menu = new Fl_Menu_Bar(0, 0, width + 100, 30);
        menu->add("File/Import Image", FL_CTRL + 'i', import_image_cb);
        menu->add("File/Quit", FL_CTRL + 'q', quit_cb);
        box = new Fl_Box(0, 30, width, height);
        box->align(FL_ALIGN_IMAGE_BACKDROP);
        auto x_input = new Fl_Value_Input(width + 45, 30, 50, 30, "x");
        x_input->callback([](Fl_Widget *w, void *data)
                          {
                            std::cout << "x: " << ((Fl_Value_Input *)w)->value() << std::endl;
                          });
        auto y_input = new Fl_Value_Input(width + 45, 60, 50, 30, "y");
        y_input->callback([](Fl_Widget *w, void *data)
                          {
                            std::cout << "y: " << ((Fl_Value_Input *)w)->value() << std::endl;
                          });
        auto z_input = new Fl_Value_Input(width + 45, 90, 50, 30, "z");
        z_input->callback([](Fl_Widget *w, void *data)
                          {
                            std::cout << "z: " << ((Fl_Value_Input *)w)->value() << std::endl;
                          });
        auto row_input = new Fl_Value_Input(width + 45, 120, 50, 30, "row");
        row_input->callback([](Fl_Widget *w, void *data)
                            {
                                std::cout << "row: " << ((Fl_Value_Input *)w)->value() << std::endl;
                            });
        auto yaw_input = new Fl_Value_Input(width + 45, 150, 50, 30, "yaw");
        yaw_input->callback([](Fl_Widget *w, void *data)
                            {
                                std::cout << "yaw: " << ((Fl_Value_Input *)w)->value() << std::endl;
                            });
        auto pitch_input = new Fl_Value_Input(width + 45, 180, 50, 30, "pitch");
        pitch_input->callback([](Fl_Widget *w, void *data)
                              {
                                std::cout << "pitch: " << ((Fl_Value_Input *)w)->value() << std::endl;
                              });
    }
};

void import_image_cb(Fl_Widget *, void *)
{
    // Show file chooser dialog
    Fl_File_Chooser chooser(".", "*.png", Fl_File_Chooser::SINGLE, "Import Image");
    chooser.show();
    while (chooser.shown())
        Fl::wait();

    // Load selected image
    std::string filename = chooser.value();
    if (filename.empty())
        return; // user cancelled
    image = new Fl_PNG_Image(filename.c_str());
    if (!image->w() || !image->h())
    {
        fl_alert("Error loading image");
        delete image;
        image = nullptr;
        return;
    }

    box->image(image);
    box->redraw();
}

void quit_cb(Fl_Widget *, void *)
{
    exit(0);
}

int main(int argc, char **argv)
{
    int height = 1080;
    int width = 1920;
    window = new Fl_Window(width + 100, height + 30, "Renderer");


    auto renderer = new Renderer();

    window->end();
    window->show(argc, argv);
    return Fl::run();
}
