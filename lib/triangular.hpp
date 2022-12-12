#pragma once
#include <vector>
#include "edge.hpp"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#define MAX3(a, b, c) (a > b ? (a > c ? a : c) : (b > c ? b : c))
#define MIN3(a, b, c) (a < b ? (a < c ? a : c) : (b < c ? b : c))

class Triangular
{   
public:
    // triangular type
    enum TriangularType {
        NORMAL,
        LINE,
        POINT,
        ERROR
    };

    bool is_active = false;
    std::vector<Vertex> vertices; // vertical points
    std::vector<Edge> edges; 
    std::vector<Edge> active_edges; 

    cv::Point3f normal; // surface normal vector
    cv::Vec3f color;

    unsigned int id;
    int dy; // y_max - y_min
    int y_max; // use to categorize the triangular
    int y_min; // use to determine whether the triangular is active
    TriangularType type = NORMAL;

    Triangular(unsigned int id, std::vector<cv::Point2i> points, std::vector<float>z_depths, cv::Vec3f color):id(id), color(color) {
        if (points.size() == 3) {
            // deal with 3 vertices are too close
            if(points[0] == points[1] && points[0] == points[2]){
                this->type = POINT;
                std::vector<cv::Point> new_points = {points[0]};
                std::vector<float> new_z_depths = {MIN3(z_depths[0], z_depths[1], z_depths[2])}; 
                this->init_vertices(new_points, new_z_depths);
            }
            else if(points[0] == points[1]){
                this->type = LINE;
                std::vector<cv::Point> new_points = {points[0], points[2]};
                std::vector<float> new_z_depths = {MIN(z_depths[0], z_depths[1]), z_depths[2]}; 
                this->init_vertices(new_points, new_z_depths);                
            }
            else if(points[1] == points[2]){
                this->type = LINE;
                std::vector<cv::Point> new_points = {points[1], points[0]};
                std::vector<float> new_z_depths = {MIN(z_depths[2], z_depths[1]), z_depths[0]}; 
                this->init_vertices(new_points, new_z_depths);                           
            }
            else if(points[0] == points[2]){
                this->type = LINE;
                std::vector<cv::Point> new_points = {points[0], points[1]};
                std::vector<float> new_z_depths = {MIN(z_depths[0], z_depths[2]), z_depths[1]}; 
                this->init_vertices(new_points, new_z_depths);                           
            }
            else{
                this->type = NORMAL;
                this->y_max = MAX3(points[0].y, points[1].y, points[2].y);
                this->y_min = MIN3(points[0].y, points[1].y, points[2].y);
                this->dy = this->y_max - this->y_min;
                this->init_vertices(points, z_depths);
                this->get_normal();
            }
            this->init_edges();
        }
        else if(points.size()==2){
            // deal with 2 vertices are too close
            if (points[0] == points[1]){
                this->type = POINT;
                std::vector<cv::Point> new_points = {points[0]};
                std::vector<float> new_z_depths = {MIN(z_depths[0], z_depths[1])}; 
                this->init_vertices(new_points, new_z_depths);
            }
            else{
                this->type = LINE;
                this->y_max = MAX(points[0].y, points[1].y);
                this->y_min = MIN(points[0].y, points[1].y);
                this->dy = this->y_max - this->y_min;
                this->init_vertices(points, z_depths);
            }
            this->init_edges();
        }
        else {
            this->type = ERROR;
        }
    }

    cv::Point3f get_normal() {
        cv::Point3f v1 = this->vertices[1].vec() - this->vertices[0].vec();
        cv::Point3f v2 = this->vertices[2].vec() - this->vertices[0].vec();
        this->normal = v1.cross(v2);
        return normal;
    }

    void init_vertices(std::vector<cv::Point> points, std::vector<float>z_depths){
        for (int i = 0; i < points.size(); i++) {
            Vertex v;
            v.point = points[i];
            v.z_depth = z_depths[i];
            this->vertices.push_back(v);
        }
    }

    void init_edges(){
        if (this->type == NORMAL) {
            if (this->vertices[0].point.y == this->vertices[1].point.y) {
                this->edges.push_back(Edge(this->vertices[0], this->vertices[2], this));
                this->edges.push_back(Edge(this->vertices[1], this->vertices[2], this));
            }
            else if (this->vertices[1].point.y == this->vertices[2].point.y) {
                this->edges.push_back(Edge(this->vertices[0], this->vertices[1], this));
                this->edges.push_back(Edge(this->vertices[0], this->vertices[2], this));
            }
            else if (this->vertices[0].point.y == this->vertices[2].point.y) {
                this->edges.push_back(Edge(this->vertices[0], this->vertices[1], this));
                this->edges.push_back(Edge(this->vertices[1], this->vertices[2], this));
            }
            else {
                this->edges.push_back(Edge(this->vertices[0], this->vertices[1], this));
                this->edges.push_back(Edge(this->vertices[1], this->vertices[2], this));
                this->edges.push_back(Edge(this->vertices[2], this->vertices[0], this));
            }
        }
        else if (this->type == LINE) {
            this->edges.push_back(Edge(this->vertices[0], this->vertices[1], this));
        }
        else if (this->type == POINT){
            this->edges.push_back(Edge(this->vertices[0], this->vertices[0], this));
        }
        std::sort(this->edges.begin(), this->edges.end());

        // std::cout << std::endl;
        // for (auto &edge : this->edges) {
        //     edge.info();
        // }
    }

    // z_depth increment while y decrease 1
    float dz_dy(){
        return this->normal.y / this->normal.z;
    }

    // z_depth increment while x increase 1
    float dz_dx(){
        return -this->normal.x / this->normal.z;
    }

    // activate triangular, add intersected edges to active_edges
    EdgePair activate(int y_now) {
        this->is_active = true;
        Edge* remain_edge = NULL;
        // return the EdgePair intersected with the scanline at y_now
        for (auto &edge : this->edges) {
            if (edge.activate(y_now)) {
                this->active_edges.push_back(edge);
            }
            else{
                remain_edge = &edge;
            }
        }
        
        if (this->active_edges.size() == 2) {
            if (remain_edge != NULL){
                EdgePair ep(this->active_edges[0], this->active_edges[1], remain_edge, this);
                return ep;
            }
            else {
                EdgePair ep(this->active_edges[0], this->active_edges[1], this);
                return ep;
            }
        }
        else{
            EdgePair ep(this->active_edges[0], this);
            return ep;
        }
    }

    void deactivate() {
        this->is_active = false;
    }    

    void info(){
        std::cout << "Triangular " << this->id << ": active_edges.size() = " << this->active_edges.size()
                << ", max_y=" << MAX3(vertices[0].point.y, vertices[1].point.y, vertices[2].point.y) 
                << ", min_y=" << MIN3(vertices[0].point.y, vertices[1].point.y, vertices[2].point.y)
                << ", (x,y) = (" << vertices[0].point.x << "," << vertices[0].point.y << "), (" << vertices[1].point.x << "," << vertices[1].point.y << "), (" << vertices[2].point.x << "," << vertices[2].point.y << ")"
                << std::endl;

        // echo edges info 
        // if (this->type == NORMAL){
        //     std::cout << "\nTriangular " << this->id;
        //     for (auto &edge : this->edges)
        //         edge.info();
        // }
    }
};

