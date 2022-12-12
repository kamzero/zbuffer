#include <iostream>
#include <vector>
#include <limits>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h> // aiMaterial class
#include <GL/gl.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

class Renderer{
public:
    string mesh_path = "/nas/3D-MAE/ShapeNet/model_v2/ShapeNetCore.v2/02691156/1bdeb4aaa0aaea4b4f95630cc18536e0/models/model_normalized.obj";  
    
    const aiScene* scene;

    cv::Matx44f transform_matrix;

    cv::Matx33f K;

    cv::Mat rvec;
    cv::Matx<float, 3, 1> tvec;

    Renderer(){
        transform_matrix = cv::Matx44f(0.6330, -0.1116, -0.7660,  0.0468,
                -0.2268,  0.9194, -0.3214, -0.0677,
                0.7402,  0.3772,  0.5567, -1.4118,
                -0.0000,  0.0000, -0.0000,  1.0000);

        K = cv::Matx33f(2.66666667e+03, 0.00000000e+00, 9.60000000e+02,
            0.00000000e+00, 2.66666667e+03, 5.40000000e+02,
            0.00000000e+00, 0.00000000e+00, 1.00000000e+00);

        cv::Rodrigues(transform_matrix.get_minor<3,3>(0,0), rvec);
        tvec = transform_matrix.get_minor<3,1>(0,3);
    }

    ~Renderer(){
    }

    void load_mesh(){
        // Create an importer instance
        Assimp::Importer importer;
        // Load the mesh using the importer
        scene = importer.ReadFile(mesh_path, aiProcess_Triangulate);

        // Check if the mesh was loaded successfully
        if (!scene)
        {
            // Handle error
            printf("Error: %s", importer.GetErrorString());
        }
        else{
            printf("Success: %d meshes loaded\n", scene->mNumMeshes);
        }
    }

    void render_mesh(){
        cv::Mat image(SCREEN_HEIGHT,SCREEN_WIDTH, CV_32FC3);

        unsigned int numMeshes = 0;
        // Iterate over the mesh's meshes
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            printf("mesh %d: %d faces\n", i, scene->mMeshes[i]->mNumFaces);
            numMeshes += scene->mMeshes[i]->mNumFaces;

            // Get the current mesh
            const aiMesh* mesh = scene->mMeshes[i];
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
            {
                const aiFace& face = mesh->mFaces[j];
                
                aiColor4D diffuse;
                // Get the color of the face, if it exists
                if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
                {
                    // Use the faceColor variable to access the color of the face
                    // For example, you can print it to the console like this:
                    std::cout << "Face " << j << " color: (" << diffuse.r << ", " << diffuse.g << ", " << diffuse.b << ", " << diffuse.a << ")" << std::endl;
                }
                printf("mesh %d: %d faces\n", i, scene->mMeshes[i]->mNumFaces);

                // Iterate over the face's vertices
                for (unsigned int k = 0; k < face.mNumIndices; ++k)
                {
                    // Get the current vertex
                    const aiVector3D& vertex = mesh->mVertices[face.mIndices[k]];
                    cv::Vec3f point3D(vertex.x, vertex.y, vertex.z);

                    // Project the 3D point to the image plane
                    std::vector<cv::Point2f> point2D;
                    cv::projectPoints(point3D, rvec, tvec, K, cv::Mat(), point2D);
                    cv::Point2i point2D_int(point2D[0].x, point2D[0].y);

                    cv::Vec<float, 3> color =  cv::Vec<float, 3>(diffuse.r*255, diffuse.g*255, diffuse.b*255);
                    image.at<Vec3f>(point2D_int) = color;

                    // Set the vertex color
                    // glColor3f(diffuse.r, diffuse.g, diffuse.b);
                }
            }
        }

        printf("numMeshes: %d\n", numMeshes);
        
        // Save the image to a file
        cv::imwrite("image.png", image);
    }
};



void render_mesh(std::string mesh_path, const aiScene* scene){

}

double compute_z(int x, int y){
    return 0.0;
}

void scanline_zbuffer(){
    // Create a Z buffer
    std::vector<std::vector<float>> z_buffer(SCREEN_WIDTH, std::vector<float>(SCREEN_HEIGHT, std::numeric_limits<float>::infinity()));

    // Scan each row of the screen
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        // Scan each column of the screen
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            // Compute the Z value of the current pixel
            float z = compute_z(x, y);

            // Check if the current pixel is closer to the viewer than the pixel in the Z buffer
            if (z < z_buffer[x][y])
            {
                // Update the pixel color and Z value in the Z buffer
                // set_pixel_color(x, y, compute_color(x, y));
                z_buffer[x][y] = z;
            }
        }
    }
}


int main()
{
    std::cout << "---------------------zbuffer: scanline version---------------------" << std::endl;
    Renderer* renderer = new Renderer();
    // load 3D mesh
    renderer->load_mesh();

    // project 3D mesh to 2D image plane
    renderer->render_mesh();

    // rasterize 2D image plane
    scanline_zbuffer();

    return 0;
}
