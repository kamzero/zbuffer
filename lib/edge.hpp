#pragma once
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "triangular.hpp"

class Triangular;

struct Vertex {
    cv::Point2i point;
    float z_depth;

    cv::Vec3f vec(){
        return cv::Vec3f(point.x, point.y, z_depth);
    }
};

struct Edge
{
    int y_max; // use to categorize the edge
    int y_min; // use to determine whether the edge is active
    int x_start; // x coordinate at y_max
    float z_start; // z coordinate at y_max
    float x_now; // left x coordinate in the current scanline
    const Triangular* triangular; // which triangular it belongs to
    float dx_dy; // (x_max - x_min) / (y_max - y_min)

    bool is_activate = false;
    Vertex v0, v1; // two vertices

    bool operator< (const Edge& edge) const {
        return (this->y_max > edge.y_max) || (this->y_max == edge.y_max && this->x_start < edge.x_start) || (this->y_max == edge.y_max && this->x_start == edge.x_start && this->dx_dy > edge.dx_dy);
    }

    Edge& operator= (const Edge& edge) {
        this->y_max = edge.y_max;
        this->y_min = edge.y_min;
        this->x_start = edge.x_start;
        this->z_start = edge.z_start;
        this->x_now = edge.x_now;
        this->triangular = edge.triangular;
        this->dx_dy = edge.dx_dy;
        this->is_activate = edge.is_activate;
        this->v0 = edge.v0;
        this->v1 = edge.v1;
        return *this;
    }

    Edge(Vertex va, Vertex vb, const Triangular* triangular): triangular(triangular){
        if (va.point.y > vb.point.y) {
            this->v0 = va;
            this->v1 = vb;
        }
        else {
            this->v0 = vb;
            this->v1 = va;
        }
        this->y_max = this->v0.point.y;
        this->y_min = this->v1.point.y;
        this->x_start = this->v0.point.x;
        this->dx_dy = (double)(this->v1.point.x - this->v0.point.x) / (double)(this->v1.point.y - this->v0.point.y);
        this->z_start = this->v0.z_depth;
    }

    bool intersect(int y_now) {
        if (y_now <= this->y_max && y_now >= this->y_min) return true;
        else return false;
    }

    float set_x_now(int y_now) {
        this->x_now = this->x_start + (y_now - this->y_max) * this->dx_dy;
        return this->x_now;
    }

    bool start_at(int y_now){
        return (y_now == this->y_max);
    }

    // try to activate the edge when scanline reaches y_max
    bool activate(unsigned int y_now) {
        if (y_now == this->y_max){
            this->is_activate = true;
            this->x_now = this->x_start;
            return true;
        }
        else return false;
    }

    void deactivate() {
        this->is_activate = false;
    }

    // update while scanline increases
    bool update(int y_now) {
        this->x_now -= this->dx_dy; 
        if(y_now <= this->y_min) {
            deactivate();
            return false;
        }
        else return true;
    }

    void info(){
        std::cout << this << " xnow=" << this->x_now << " v0=" << this->v0.point << ",v1=" << this->v1.point << ",y_max=" << this->y_max << "  "; 
    }

};


struct EdgePair{
    enum EdgePairType {
        PAIR,
        SINGLE
    };

    EdgePairType type;
    const Triangular* triangular; // which triangular it belongs to
    Edge* left = nullptr;
    Edge* right = nullptr;
    Edge* remain = nullptr;
    
    EdgePair(Edge* el, const Triangular* triangular): left(el), triangular(triangular), type(SINGLE) {
        this->right = this->left;
    }
    EdgePair(Edge* el, Edge* er, const Triangular* triangular): left(el), right(er), triangular(triangular), type(PAIR) {}


    EdgePair(Edge* el, Edge* er, Edge* pr, const Triangular* triangular): left(el), right(er), remain(pr), triangular(triangular), type(PAIR) {}

    EdgePair& operator= (const EdgePair& edge_pair) {
        this->type = edge_pair.type;
        this->triangular = edge_pair.triangular;
        this->left = edge_pair.left;
        this->right = edge_pair.right;
        this->remain = edge_pair.remain;
        return *this;
    }

    bool update(int y_now){
        if (this->type == SINGLE) {
            return this->left->update(y_now);
        }
        else if (this->remain == nullptr) {
            return this->left->update(y_now) && this->right->update(y_now);
        }
        else if(this->left->update(y_now) && this->right->update(y_now)) return true;
        else {
            // activate the remain edge
            this->remain->set_x_now(y_now);
            this->remain->is_activate = true;
            // update the old edge
            if (this->left->y_min == y_now) {
                this->left = this->remain;
            }
            else if (this->right->y_min == y_now) {
                this->right = this->remain;
            }
            this->remain = nullptr;
            return true;
        }
        return false;
    }

    void info(){
        if (this->type == PAIR) {
            std::cout << "\nleft: ";
            this->left->info();
            std::cout << " right: ";
            this->right->info();
        }
    }
};