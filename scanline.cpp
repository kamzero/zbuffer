#include <iostream>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h> // aiMaterial class
#include <GL/gl.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "lib/triangular.hpp"
#include "lib/triangular_table.hpp"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

cv::Matx44f transform_matrix(0.6330, -0.1116, -0.7660, 0.0468,
                             -0.2268, 0.9194, -0.3214, -0.0677,
                             0.7402, 0.3772, 0.5567, -1.4118,
                             -0.0000, 0.0000, -0.0000, 1.0000);

cv::Matx33f K(2.66666667e+03, 0.00000000e+00, 9.60000000e+02,
              0.00000000e+00, 2.66666667e+03, 5.40000000e+02,
              0.00000000e+00, 0.00000000e+00, 1.00000000e+00);

void load_mesh_init(std::string mesh_path, TriangularTable& triangular_table)
{
    // Create an importer instance
    Assimp::Importer importer;
    cv::Mat image(SCREEN_HEIGHT, SCREEN_WIDTH, CV_32FC3);
    // Load the mesh using the importer
    const aiScene *scene = importer.ReadFile(mesh_path, aiProcess_Triangulate);

    // Get the vector of transformation matrices
    cv::Mat rvec;
    cv::Rodrigues(transform_matrix.get_minor<3, 3>(0, 0), rvec);
    cv::Matx<float, 3, 1> tvec = transform_matrix.get_minor<3, 1>(0, 3);

    // Check if the mesh was loaded successfully
    if (!scene)
        printf("Error: %s", importer.GetErrorString());
    else
        printf("Successfully loaded\n");

    // Iterate over the mesh's meshes
    unsigned int numMeshes = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        // Get the current mesh and its material
        const aiMesh *mesh = scene->mMeshes[i];
        const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        for (unsigned int j = 0; j < mesh->mNumFaces; ++j, numMeshes+=1)
        {
            const aiFace &face = mesh->mFaces[j];
            aiColor4D diffuse;
            // Get the color of the face, if it exists
            if (AI_SUCCESS != aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            {
                // If the color doesn't exist, set it to white
                diffuse = aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
            }
            cv::Vec3f color = cv::Vec3f(diffuse.r * 255, diffuse.g * 255, diffuse.b * 255);
            std::vector<cv::Point2i> points;
            std::vector<float> z_depths;
            // Iterate over the face's vertices
            for (unsigned int k = 0; k < face.mNumIndices; ++k)
            {
                // Get the current vertex
                const aiVector3D &vertex = mesh->mVertices[face.mIndices[k]];
                cv::Vec3f point3D(vertex.x, vertex.y, vertex.z);
                cv::Vec4f point3D_homo(vertex.x, vertex.y, vertex.z, 1);

                // transform to camera frame, obtain z-depth
                cv::Vec4f point_camera = transform_matrix * point3D_homo;
                float z_depth = point_camera[3];

                // Project the 3D point to the image plane
                std::vector<cv::Point2f> point2D;
                cv::projectPoints(point3D, rvec, tvec, K, cv::Mat(), point2D);
                cv::Point2i point2D_int(point2D[0].x, point2D[0].y);

                // check if the point is in the image
                if (point2D_int.x < 0 || point2D_int.x >= SCREEN_WIDTH || point2D_int.y < 0 || point2D_int.y >= SCREEN_HEIGHT || z_depth < 0){
                    std::cout << "point out of image" << std::endl;
                    // TODO: deal with this case
                    continue;
                }

                points.push_back(point2D_int);
                z_depths.push_back(z_depth);

                image.at<cv::Vec3f>(point2D_int) = color;
            }
            Triangular triangular = Triangular(numMeshes, points, z_depths, color);
            triangular_table.add_triangular(triangular);
        }
    }

    triangular_table.summary();

    // Save the image to a file
    cv::imwrite("image.png", image);
}

void scanline_zbuffer(TriangularTable& triangular_table)
{
    cv::Mat frame_buffer(SCREEN_HEIGHT, SCREEN_WIDTH, CV_32FC3);
    cv::Mat z_buffer(SCREEN_HEIGHT, SCREEN_WIDTH, CV_32FC1);

    // Scan each row of the screen
    for (int y = SCREEN_HEIGHT-1; y >= 0; y--)
    {
        // scan triangular_table, add EdgePair from new Triangular
        triangular_table.update_table(y);

        for (auto it = triangular_table.active_edgepairs.begin(); it != triangular_table.active_edgepairs.end();)
        {
            // update activate_edgepairs, increase x_now of each edge
            if(it->update(y)){ // if the edgepair is still active
                it++;
            }
            else{ // if the edgepair is not active, remove inactivate edgepair
                it = triangular_table.active_edgepairs.erase(it);
            }
        }

        // Scan each column of the screen
        // for (int x = 0; x < SCREEN_WIDTH; ++x)
        // {
        //     cv::Point2i p(x,y);
        //     // Compute the Z value of the current pixel
        //     float z = 0;

        //     // Check if the current pixel is closer to the viewer than the pixel in the Z buffer
        //     if (z < z_buffer.at<float>(p))
        //     {
        //         // Update the pixel color and Z value in the Z buffer
        //         // set_pixel_color(x, y, compute_color(x, y));
        //         z_buffer.at<float>(p) = z;
        //     }
        // }
    }
}

int main(int argc, char **argv) 
{
    // TODO: bulid triangle_table, edge table + scanline z-buffer 
    std::cout << "---------------------zbuffer: scanline version---------------------" << std::endl;
    TriangularTable triangular_table = TriangularTable(SCREEN_HEIGHT);

    std::cout << "---------------------stage 1: load mesh & project------------------" << std::endl;

    std::string mesh_path = "/nas/3D-MAE/ShapeNet/model_v2/ShapeNetCore.v2/02691156/1bdeb4aaa0aaea4b4f95630cc18536e0/models/model_normalized.obj"; // plane
    // load 3D mesh & create
    load_mesh_init(mesh_path, triangular_table);

    // rasterize 2D image using scanline z-buffer
    std::cout << "---------------------stage 2: rasterize mesh-----------------------" << std::endl;
    scanline_zbuffer(triangular_table);

    return 0;
}
