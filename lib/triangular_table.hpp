#pragma once
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "triangular.hpp"
#include "edge.hpp"

class TriangularTable
{   
public:
    std::vector<std::vector<Triangular>> triangulars;
    // std::vector<Triangular> active_triangulars;
    std::vector<EdgePair> active_edgepairs;
    int height;

    TriangularTable(int height): height(height){
        this->triangulars.resize(height);
    }

    void summary(){
        int total_count = 0;
        int normal_count = 0;
        int line_count = 0;
        int point_count = 0;
        int error_count = 0;
        for(auto triangulars: this->triangulars){
            for(auto triangular: triangulars){
                // triangular.info();
                total_count++;
                switch (triangular.type)
                {
                case Triangular::NORMAL:
                    normal_count++;
                    break;
                case Triangular::LINE:
                    line_count++;
                    break;
                case Triangular::POINT:
                    point_count++;
                    break;
                case Triangular::ERROR:
                    error_count++;
                    break;
                default:
                    break;
                }
            }
        }
        std::cout << "Meshes Summary: Total=" << total_count << ", Normal=" << normal_count << ", Line=" << line_count << ", Point=" << point_count << ", Error=" << error_count << std::endl;
    }

    void add_triangular(Triangular& triangular) {
        this->triangulars[triangular.y_max].push_back(triangular);
    }

    void update_table(unsigned int y_now){
        for (auto& triangular: this->triangulars[y_now]) {
            if(triangular.type != Triangular::NORMAL) continue;
            auto ep = triangular.activate(y_now);
            // this->active_triangulars.push_back(triangular);
            this->active_edgepairs.push_back(ep);
        }
    }

};

