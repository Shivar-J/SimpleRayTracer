#include "utils.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "moving_sphere.h"
#include "aarect.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <thread>
#include <vector>
#include <execution>

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    if (depth <= 0) {
        return color(0, 0, 0);
    }

    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list random_scene() {
    hittable_list world;

    auto pertext = make_shared<noise_texture>(4);
   
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));
    
    return world;
}

hittable_list scene() {
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

hittable_list earth() {
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

    return hittable_list(globe);
}

hittable_list two_spheres() {
    hittable_list objects;

    auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    return objects;
}

hittable_list simple_light() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(4);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(color(1,1,1))));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(color(0, 0.7, 1.0))));

    auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
    //objects.add(make_shared<sphere>(point3(0, 7, 0), 2, difflight));
    objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

    return objects;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 1110, 0, 1110, 1110, green));
    objects.add(make_shared<yz_rect>(0, 1110, 0, 1110, 0, red));
    objects.add(make_shared<xz_rect>(426, 686, 454, 664, 1108, light));
    objects.add(make_shared<xz_rect>(0, 1110, 0, 1110, 0, white));
    objects.add(make_shared<xz_rect>(0, 1110, 0, 1110, 1110, white));
    objects.add(make_shared<xy_rect>(0, 1110, 0, 1110, 1110, white));

    return objects;
}

int main() {
    // image
    //const auto aspect_ratio = 16.0 / 9.0;
    const auto aspect_ratio = 1.0;
    const int image_width = 600;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 1000;
    const int max_depth = 100;
    const int pixelCount = image_width * image_height;

    width = image_width;
    height = image_height;
    renderBuffer = new color[pixelCount];

    // world
    hittable_list world = cornell_box();

    // camera
    point3 lookfrom(556, 556, -1600);
    point3 lookat(556, 556, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperature = 0.0;
    //color background(0.7, 0.8, 1.0);
    color background(0.00, 0.00, 0.00);

    vec3* image = new vec3[pixelCount];
    memset(&image[0], 0, pixelCount * sizeof(vec3));

    camera cam(lookfrom, lookat, vup, 35, aspect_ratio, aperature, dist_to_focus, 0.0, 0.0);

    // render
    std::ofstream file;
    file.open("image.ppm", std::ios::out | std::ios::binary);
    file << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    std::vector<int> horizontalIter, verticalIter;
    horizontalIter.resize(image_width);
    verticalIter.resize(image_height);
    for (int i = 0; i < image_width; i++) {
        horizontalIter[i] = i;
    }
    for (int i = 0; i < image_height; i++) {
        verticalIter[i] = i;
    }

#define MT 1
#if MT
    //using for_each loop to utilize paralization
    std::for_each(std::execution::par, verticalIter.begin(), verticalIter.end(), [&](int y) {
        int total = verticalIter.size() - y;
        std::cerr << "\rScanlines remaining: " << total << ' ' << std::flush;
        std::for_each(std::execution::par, horizontalIter.begin(), horizontalIter.end(), [y, image_height, cam, world, &file, background](int x) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; s++) {
                auto u = (x + random_double()) / (image_width - 1);
                auto v = (y + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);

                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(x, y, pixel_color, samples_per_pixel);
            });
        });

    WriteBufferToFile(file);
#else
    //scans from height to width (top to bottom, left to right)
    //hence the first loop being height then width
    for (int y = image_height - 1; y >= 0; --y) {
        std::cerr << "\rScanlines remaining: " << y << ' ' << std::flush;
        for (int x = 0; x < image_width; ++x) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; s++) {
                auto u = (x + random_double()) / (image_width - 1);
                auto v = (y + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(file, pixel_color, samples_per_pixel);
        }
    } 
#endif
    std::cerr << "\nDone.\n";
    file.close();
    return 0;
}