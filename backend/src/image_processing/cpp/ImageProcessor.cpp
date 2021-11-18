//
// Created by ThinkPad41 on 10/10/2021.
//


#include "../header/ImageProcessor.h"

ImageProcessor::ImageProcessor(const vector<shared_ptr<ImgProcessingComponent>> &components) {
    for(auto & component : components){
        this->components.push_back(component);
    }
}
void ImageProcessor::execute(CallBackFunction func) {
    this->callback_func = func;
    this->callback_func("Starting Image Processor");
    for(auto  & component : this->components){
        component->execute(std::bind(&ImageProcessor::my_callback, this, placeholders::_1));
    }
    this->callback_func("Image Processing Done!!!");
}

void ImageProcessor::my_callback(string str) {
    this->callback_func("ImageProcessor->" + str);
}
