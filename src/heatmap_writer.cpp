#include "heatmap_writer.h"
#include "image_util.h"

#include <boost/algorithm/clamp.hpp>

#include <unordered_map>

using namespace wotreplay;
using boost::algorithm::clamp;

static float stamp_default_4_data[] = {
    0.0f      , 0.0f      , 0.1055728f, 0.1753789f, 0.2f, 0.1753789f, 0.1055728f, 0.0f      , 0.0f      ,
    0.0f      , 0.1514719f, 0.2788897f, 0.3675445f, 0.4f, 0.3675445f, 0.2788897f, 0.1514719f, 0.0f      ,
    0.1055728f, 0.2788897f, 0.4343146f, 0.5527864f, 0.6f, 0.5527864f, 0.4343146f, 0.2788897f, 0.1055728f,
    0.1753789f, 0.3675445f, 0.5527864f, 0.7171573f, 0.8f, 0.7171573f, 0.5527864f, 0.3675445f, 0.1753789f,
    0.2f      , 0.4f      , 0.6f      , 0.8f      , 1.0f, 0.8f      , 0.6f      , 0.4f      , 0.2f      ,
    0.1753789f, 0.3675445f, 0.5527864f, 0.7171573f, 0.8f, 0.7171573f, 0.5527864f, 0.3675445f, 0.1753789f,
    0.1055728f, 0.2788897f, 0.4343146f, 0.5527864f, 0.6f, 0.5527864f, 0.4343146f, 0.2788897f, 0.1055728f,
    0.0f      , 0.1514719f, 0.2788897f, 0.3675445f, 0.4f, 0.3675445f, 0.2788897f, 0.1514719f, 0.0f      ,
    0.0f      , 0.0f      , 0.1055728f, 0.1753789f, 0.2f, 0.1753789f, 0.1055728f, 0.0f      , 0.0f      ,
};

static unsigned char mixed_data[] = {
    0, 0, 0, 0, 94, 79, 162, 0, 93, 79, 162, 7, 93, 80, 162, 14, 92, 80, 163, 22, 92, 81, 163, 29, 91, 81, 164, 37, 91, 82, 164, 44, 90, 82, 164, 52, 90, 83, 165, 59, 89, 83, 165, 67, 89, 84, 166, 74, 88, 84, 166, 82, 88, 85, 166, 89, 87, 85, 167, 97, 87, 86, 167, 104, 86, 86, 167, 112, 86, 87, 168, 119, 85, 87, 168, 127, 85, 88, 168, 134, 84, 88, 169, 141, 84, 89, 169, 149, 83, 89, 169, 156, 83, 90, 170, 164, 83, 90, 170, 171, 82, 91, 170, 179, 82, 91, 171, 186, 81, 92, 171, 194, 81, 92, 171, 201, 80, 93, 172, 209, 80, 93, 172, 216, 79, 94, 172, 224, 79, 94, 172, 231, 78, 95, 173, 239, 78, 95, 173, 246, 77, 95, 173, 254, 77, 96, 174, 255, 76, 96, 174, 255, 76, 97, 174, 255, 75, 97, 174, 255, 75, 98, 175, 255, 74, 98, 175, 255, 74, 99, 175, 255, 73, 99, 175, 255, 73, 100, 176, 255, 72, 100, 176, 255, 72, 101, 176, 255, 72, 101, 176, 255, 71, 101, 176, 255, 71, 102, 177, 255, 70, 102, 177, 255, 70, 103, 177, 255, 69, 103, 177, 255, 60, 115, 183, 255, 60, 115, 183, 255, 59, 116, 183, 255, 59, 116, 183, 255, 58, 117, 184, 255, 58, 117, 184, 255, 58, 118, 184, 255, 57, 118, 184, 255, 57, 118, 184, 255, 56, 119, 184, 255, 56, 119, 185, 255, 56, 120, 185, 255, 55, 120, 185, 255, 55, 121, 185, 255, 55, 121, 185, 255, 54, 121, 185, 255, 54, 122, 186, 255, 54, 122, 186, 255, 53, 123, 186, 255, 53, 123, 186, 255, 53, 124, 186, 255, 52, 124, 186, 255, 52, 124, 186, 255, 52, 125, 186, 255, 52, 125, 187, 255, 51, 126, 187, 255, 51, 126, 187, 255, 51, 126, 187, 255, 51, 127, 187, 255, 50, 127, 187, 255, 50, 128, 187, 255, 50, 128, 187, 255, 50, 128, 187, 255, 50, 129, 187, 255, 50, 129, 188, 255, 50, 130, 188, 255, 49, 130, 188, 255, 49, 130, 188, 255, 49, 131, 188, 255, 49, 131, 188, 255, 49, 132, 188, 255, 49, 132, 188, 255, 49, 132, 188, 255, 49, 133, 188, 255, 49, 133, 188, 255, 49, 133, 188, 255, 49, 134, 188, 255, 49, 134, 188, 255, 49, 135, 188, 255, 49, 135, 188, 255, 49, 135, 188, 255, 49, 136, 189, 255, 47, 136, 189, 255, 46, 137, 189, 255, 45, 138, 189, 255, 43, 138, 189, 255, 42, 139, 190, 255, 41, 139, 190, 255, 39, 140, 190, 255, 38, 140, 190, 255, 36, 141, 190, 255, 35, 141, 190, 255, 33, 142, 190, 255, 31, 142, 190, 255, 30, 143, 190, 255, 28, 143, 190, 255, 26, 144, 191, 255, 24, 145, 191, 255, 22, 145, 191, 255, 20, 146, 191, 255, 17, 146, 191, 255, 15, 147, 191, 255, 12, 147, 191, 255, 10, 148, 191, 255, 10, 148, 191, 255, 10, 149, 191, 255, 10, 149, 191, 255, 10, 150, 191, 255, 10, 150, 190, 255, 10, 151, 190, 255, 10, 151, 190, 255, 10, 152, 190, 255, 10, 152, 190, 255, 10, 153, 190, 255, 10, 153, 190, 255, 10, 154, 190, 255, 10, 154, 190, 255, 10, 155, 190, 255, 10, 155, 189, 255, 10, 156, 189, 255, 10, 156, 189, 255, 10, 157, 189, 255, 10, 157, 189, 255, 10, 158, 189, 255, 10, 158, 188, 255, 10, 158, 188, 255, 10, 159, 188, 255, 10, 159, 188, 255, 10, 160, 188, 255, 10, 160, 187, 255, 10, 161, 187, 255, 10, 161, 187, 255, 20, 173, 182, 255, 22, 174, 182, 255, 25, 174, 181, 255, 28, 175, 181, 255, 30, 175, 181, 255, 33, 176, 180, 255, 35, 176, 180, 255, 37, 176, 180, 255, 39, 177, 180, 255, 41, 177, 179, 255, 43, 178, 179, 255, 45, 178, 179, 255, 46, 179, 178, 255, 48, 179, 178, 255, 50, 179, 178, 255, 51, 180, 177, 255, 53, 180, 177, 255, 54, 181, 177, 255, 56, 181, 176, 255, 58, 182, 176, 255, 59, 182, 176, 255, 61, 182, 175, 255, 62, 183, 175, 255, 64, 183, 175, 255, 65, 184, 174, 255, 66, 184, 174, 255, 68, 184, 174, 255, 69, 185, 173, 255, 71, 185, 173, 255, 72, 186, 173, 255, 74, 186, 172, 255, 75, 186, 172, 255, 76, 187, 171, 255, 78, 187, 171, 255, 79, 187, 171, 255, 80, 188, 170, 255, 82, 188, 170, 255, 83, 189, 170, 255, 85, 189, 169, 255, 86, 189, 169, 255, 87, 190, 169, 255, 89, 190, 168, 255, 90, 190, 168, 255, 91, 191, 167, 255, 93, 191, 167, 255, 94, 191, 167, 255, 95, 192, 166, 255, 97, 192, 166, 255, 98, 193, 166, 255, 99, 193, 165, 255, 100, 193, 165, 255, 102, 194, 164, 255, 102, 194, 164, 255, 103, 194, 164, 255, 103, 194, 164, 255, 104, 194, 164, 255, 104, 195, 164, 255, 105, 195, 164, 255, 105, 195, 164, 255, 106, 195, 164, 255, 106, 196, 164, 255, 107, 196, 164, 255, 108, 196, 164, 255, 108, 196, 164, 255, 109, 196, 164, 255, 109, 197, 164, 255, 110, 197, 164, 255, 110, 197, 164, 255, 111, 197, 164, 255, 111, 198, 164, 255, 112, 198, 164, 255, 112, 198, 164, 255, 113, 198, 164, 255, 113, 198, 164, 255, 114, 199, 164, 255, 115, 199, 164, 255, 115, 199, 164, 255, 116, 199, 164, 255, 116, 200, 164, 255, 117, 200, 164, 255, 117, 200, 164, 255, 118, 200, 164, 255, 118, 200, 164, 255, 119, 201, 164, 255, 119, 201, 164, 255, 120, 201, 164, 255, 120, 201, 164, 255, 121, 201, 164, 255, 122, 202, 164, 255, 122, 202, 164, 255, 123, 202, 164, 255, 123, 202, 164, 255, 124, 203, 164, 255, 124, 203, 164, 255, 125, 203, 164, 255, 125, 203, 164, 255, 126, 203, 164, 255, 126, 204, 164, 255, 127, 204, 164, 255, 127, 204, 163, 255, 128, 204, 163, 255, 129, 204, 163, 255, 143, 210, 163, 255, 143, 210, 163, 255, 144, 210, 163, 255, 144, 211, 163, 255, 145, 211, 163, 255, 146, 211, 163, 255, 146, 211, 163, 255, 147, 212, 163, 255, 147, 212, 163, 255, 148, 212, 163, 255, 148, 212, 163, 255, 149, 212, 163, 255, 149, 213, 163, 255, 150, 213, 163, 255, 150, 213, 163, 255, 151, 213, 163, 255, 151, 213, 163, 255, 152, 214, 163, 255, 153, 214, 163, 255, 153, 214, 163, 255, 154, 214, 163, 255, 154, 214, 163, 255, 155, 215, 163, 255, 155, 215, 163, 255, 156, 215, 163, 255, 156, 215, 163, 255, 157, 215, 163, 255, 157, 216, 163, 255, 158, 216, 163, 255, 158, 216, 163, 255, 159, 216, 163, 255, 160, 216, 163, 255, 160, 217, 163, 255, 161, 217, 163, 255, 161, 217, 163, 255, 162, 217, 163, 255, 162, 217, 163, 255, 163, 218, 163, 255, 163, 218, 163, 255, 164, 218, 163, 255, 164, 218, 163, 255, 165, 218, 163, 255, 166, 219, 163, 255, 166, 219, 163, 255, 167, 219, 163, 255, 167, 219, 163, 255, 168, 219, 163, 255, 168, 220, 163, 255, 169, 220, 163, 255, 169, 220, 163, 255, 170, 220, 163, 255, 170, 220, 163, 255, 171, 221, 163, 255, 171, 221, 163, 255, 172, 221, 163, 255, 172, 221, 163, 255, 172, 222, 163, 255, 173, 222, 163, 255, 173, 222, 163, 255, 173, 222, 163, 255, 174, 222, 163, 255, 174, 223, 163, 255, 175, 223, 163, 255, 175, 223, 163, 255, 175, 223, 162, 255, 176, 223, 162, 255, 176, 224, 162, 255, 177, 224, 162, 255, 177, 224, 162, 255, 177, 224, 162, 255, 178, 224, 162, 255, 178, 225, 162, 255, 179, 225, 162, 255, 179, 225, 162, 255, 179, 225, 162, 255, 180, 225, 161, 255, 180, 226, 161, 255, 181, 226, 161, 255, 181, 226, 161, 255, 182, 226, 161, 255, 182, 226, 161, 255, 182, 227, 161, 255, 183, 227, 161, 255, 183, 227, 161, 255, 184, 227, 161, 255, 184, 227, 160, 255, 185, 228, 160, 255, 185, 228, 160, 255, 185, 228, 160, 255, 186, 228, 160, 255, 186, 228, 160, 255, 187, 229, 160, 255, 187, 229, 160, 255, 188, 229, 160, 255, 188, 229, 160, 255, 189, 229, 159, 255, 189, 230, 159, 255, 189, 230, 159, 255, 190, 230, 159, 255, 190, 230, 159, 255, 191, 230, 159, 255, 191, 231, 159, 255, 192, 231, 159, 255, 204, 236, 156, 255, 205, 236, 156, 255, 205, 236, 156, 255, 205, 236, 156, 255, 206, 236, 156, 255, 206, 237, 156, 255, 207, 237, 156, 255, 207, 237, 156, 255, 208, 237, 156, 255, 208, 237, 155, 255, 209, 238, 155, 255, 209, 238, 155, 255, 210, 238, 155, 255, 210, 238, 155, 255, 211, 238, 155, 255, 211, 238, 155, 255, 212, 239, 155, 255, 212, 239, 155, 255, 213, 239, 155, 255, 213, 239, 154, 255, 214, 239, 154, 255, 214, 240, 154, 255, 215, 240, 154, 255, 215, 240, 154, 255, 216, 240, 154, 255, 216, 240, 154, 255, 217, 240, 154, 255, 217, 241, 154, 255, 218, 241, 154, 255, 218, 241, 153, 255, 219, 241, 153, 255, 219, 241, 153, 255, 220, 241, 153, 255, 220, 242, 153, 255, 221, 242, 153, 255, 221, 242, 153, 255, 222, 242, 153, 255, 222, 242, 153, 255, 223, 242, 153, 255, 223, 243, 153, 255, 224, 243, 152, 255, 224, 243, 152, 255, 225, 243, 152, 255, 225, 243, 152, 255, 226, 243, 152, 255, 227, 244, 152, 255, 227, 244, 152, 255, 228, 244, 152, 255, 228, 244, 152, 255, 229, 244, 152, 255, 229, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 152, 255, 230, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 153, 255, 231, 244, 154, 255, 231, 244, 154, 255, 231, 244, 154, 255, 232, 244, 154, 255, 232, 244, 154, 255, 232, 244, 154, 255, 232, 244, 154, 255, 232, 244, 154, 255, 232, 244, 154, 255, 232, 244, 155, 255, 232, 244, 155, 255, 232, 244, 155, 255, 232, 244, 155, 255, 233, 244, 155, 255, 233, 244, 155, 255, 233, 244, 155, 255, 233, 244, 155, 255, 233, 244, 156, 255, 233, 244, 156, 255, 233, 244, 156, 255, 233, 244, 156, 255, 233, 244, 156, 255, 233, 244, 156, 255, 233, 244, 156, 255, 233, 244, 156, 255, 234, 244, 156, 255, 234, 244, 157, 255, 234, 244, 157, 255, 234, 244, 157, 255, 234, 244, 157, 255, 234, 244, 157, 255, 234, 244, 157, 255, 234, 244, 157, 255, 236, 243, 160, 255, 237, 243, 160, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 161, 255, 237, 243, 162, 255, 237, 243, 162, 255, 237, 243, 162, 255, 238, 243, 162, 255, 238, 243, 162, 255, 238, 243, 162, 255, 238, 243, 162, 255, 238, 243, 162, 255, 238, 243, 163, 255, 238, 243, 163, 255, 238, 243, 163, 255, 238, 243, 163, 255, 238, 243, 163, 255, 238, 243, 163, 255, 238, 243, 163, 255, 239, 243, 163, 255, 239, 243, 163, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 164, 255, 239, 243, 165, 255, 239, 243, 165, 255, 239, 243, 165, 255, 240, 243, 165, 255, 240, 243, 165, 255, 240, 243, 165, 255, 240, 243, 165, 255, 240, 243, 165, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 240, 243, 166, 255, 241, 242, 166, 255, 241, 242, 166, 255, 241, 242, 166, 255, 241, 242, 166, 255, 241, 242, 165, 255, 241, 242, 165, 255, 241, 242, 165, 255, 241, 241, 165, 255, 241, 241, 165, 255, 241, 241, 164, 255, 242, 241, 164, 255, 242, 241, 164, 255, 242, 241, 164, 255, 242, 241, 164, 255, 242, 240, 163, 255, 242, 240, 163, 255, 242, 240, 163, 255, 242, 240, 163, 255, 242, 240, 163, 255, 242, 240, 162, 255, 243, 240, 162, 255, 243, 239, 162, 255, 243, 239, 162, 255, 243, 239, 162, 255, 243, 239, 161, 255, 243, 239, 161, 255, 243, 239, 161, 255, 243, 239, 161, 255, 243, 238, 161, 255, 243, 238, 161, 255, 244, 238, 160, 255, 244, 238, 160, 255, 244, 238, 160, 255, 244, 238, 160, 255, 244, 238, 160, 255, 244, 237, 159, 255, 244, 237, 159, 255, 244, 237, 159, 255, 244, 237, 159, 255, 244, 237, 159, 255, 245, 237, 158, 255, 245, 236, 158, 255, 245, 236, 158, 255, 245, 236, 158, 255, 245, 236, 158, 255, 245, 236, 157, 255, 245, 236, 157, 255, 245, 236, 157, 255, 245, 235, 157, 255, 245, 235, 157, 255, 246, 235, 157, 255, 248, 231, 152, 255, 248, 231, 152, 255, 248, 231, 151, 255, 249, 231, 151, 255, 249, 231, 151, 255, 249, 230, 151, 255, 249, 230, 151, 255, 249, 230, 151, 255, 249, 230, 150, 255, 249, 230, 150, 255, 249, 230, 150, 255, 249, 230, 150, 255, 249, 229, 150, 255, 250, 229, 150, 255, 250, 229, 149, 255, 250, 229, 149, 255, 250, 229, 149, 255, 250, 229, 149, 255, 250, 229, 149, 255, 250, 228, 148, 255, 250, 228, 148, 255, 250, 228, 148, 255, 251, 228, 148, 255, 251, 228, 148, 255, 251, 228, 148, 255, 251, 227, 147, 255, 251, 227, 147, 255, 251, 227, 147, 255, 251, 227, 147, 255, 251, 227, 147, 255, 251, 227, 147, 255, 251, 227, 147, 255, 252, 226, 146, 255, 252, 226, 146, 255, 252, 226, 146, 255, 252, 226, 146, 255, 252, 226, 146, 255, 252, 226, 146, 255, 252, 225, 145, 255, 252, 225, 145, 255, 252, 225, 145, 255, 253, 225, 145, 255, 253, 225, 145, 255, 253, 225, 145, 255, 253, 225, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 224, 144, 255, 253, 223, 143, 255, 253, 223, 143, 255, 253, 223, 142, 255, 253, 222, 142, 255, 253, 222, 141, 255, 253, 221, 141, 255, 253, 221, 141, 255, 253, 221, 140, 255, 253, 220, 140, 255, 253, 220, 139, 255, 253, 220, 139, 255, 253, 219, 138, 255, 253, 219, 138, 255, 253, 218, 138, 255, 253, 218, 137, 255, 253, 218, 137, 255, 253, 217, 136, 255, 253, 217, 136, 255, 253, 217, 135, 255, 253, 216, 135, 255, 253, 216, 135, 255, 253, 215, 134, 255, 253, 215, 134, 255, 253, 215, 133, 255, 253, 214, 133, 255, 253, 214, 133, 255, 253, 214, 132, 255, 253, 213, 132, 255, 253, 213, 131, 255, 253, 212, 131, 255, 253, 212, 131, 255, 253, 212, 130, 255, 253, 211, 130, 255, 253, 211, 129, 255, 253, 211, 129, 255, 253, 210, 129, 255, 253, 210, 128, 255, 253, 209, 128, 255, 253, 209, 127, 255, 253, 209, 127, 255, 253, 208, 127, 255, 253, 208, 126, 255, 253, 207, 126, 255, 253, 207, 125, 255, 253, 207, 125, 255, 253, 206, 125, 255, 253, 206, 124, 255, 253, 205, 124, 255, 253, 205, 123, 255, 253, 205, 123, 255, 253, 204, 123, 255, 253, 194, 113, 255, 253, 194, 113, 255, 253, 193, 112, 255, 253, 193, 112, 255, 253, 192, 112, 255, 253, 192, 111, 255, 253, 192, 111, 255, 253, 191, 110, 255, 253, 191, 110, 255, 253, 190, 110, 255, 253, 190, 109, 255, 253, 190, 109, 255, 253, 189, 109, 255, 253, 189, 108, 255, 253, 188, 108, 255, 253, 188, 108, 255, 253, 188, 107, 255, 253, 187, 107, 255, 253, 187, 107, 255, 253, 186, 106, 255, 253, 186, 106, 255, 253, 186, 106, 255, 253, 185, 105, 255, 253, 185, 105, 255, 253, 184, 105, 255, 253, 184, 104, 255, 253, 184, 104, 255, 253, 183, 104, 255, 253, 183, 103, 255, 253, 182, 103, 255, 253, 182, 103, 255, 253, 182, 102, 255, 253, 181, 102, 255, 253, 181, 102, 255, 253, 180, 101, 255, 253, 180, 101, 255, 253, 180, 101, 255, 253, 179, 100, 255, 253, 179, 100, 255, 253, 178, 100, 255, 253, 178, 100, 255, 253, 178, 99, 255, 253, 177, 99, 255, 253, 177, 99, 255, 253, 176, 98, 255, 253, 176, 98, 255, 253, 175, 98, 255, 253, 175, 98, 255, 253, 175, 97, 255, 253, 174, 97, 255, 253, 174, 97, 255, 252, 173, 96, 255, 252, 173, 96, 255, 252, 172, 96, 255, 252, 172, 95, 255, 252, 172, 95, 255, 252, 171, 95, 255, 252, 171, 94, 255, 252, 170, 94, 255, 252, 170, 94, 255, 252, 169, 93, 255, 252, 169, 93, 255, 252, 168, 93, 255, 252, 168, 92, 255, 252, 167, 92, 255, 252, 167, 92, 255, 252, 166, 91, 255, 252, 166, 91, 255, 252, 165, 91, 255, 251, 165, 90, 255, 251, 164, 90, 255, 251, 164, 90, 255, 251, 163, 89, 255, 251, 163, 89, 255, 251, 162, 89, 255, 251, 162, 89, 255, 251, 162, 88, 255, 251, 161, 88, 255, 251, 161, 88, 255, 251, 160, 87, 255, 251, 160, 87, 255, 251, 159, 87, 255, 251, 159, 87, 255, 251, 158, 86, 255, 251, 158, 86, 255, 251, 157, 86, 255, 250, 157, 85, 255, 250, 156, 85, 255, 250, 156, 85, 255, 250, 155, 85, 255, 250, 155, 84, 255, 250, 154, 84, 255, 250, 154, 84, 255, 250, 153, 84, 255, 250, 153, 83, 255, 250, 152, 83, 255, 250, 152, 83, 255, 250, 151, 83, 255, 250, 151, 82, 255, 250, 150, 82, 255, 250, 150, 82, 255, 249, 149, 82, 255, 248, 136, 75, 255, 248, 135, 75, 255, 247, 135, 75, 255, 247, 134, 75, 255, 247, 134, 74, 255, 247, 133, 74, 255, 247, 133, 74, 255, 247, 132, 74, 255, 247, 132, 74, 255, 247, 131, 73, 255, 247, 131, 73, 255, 247, 130, 73, 255, 247, 130, 73, 255, 247, 129, 72, 255, 247, 129, 72, 255, 247, 128, 72, 255, 246, 128, 72, 255, 246, 127, 72, 255, 246, 126, 71, 255, 246, 126, 71, 255, 246, 125, 71, 255, 246, 125, 71, 255, 246, 124, 71, 255, 246, 124, 71, 255, 246, 123, 70, 255, 246, 123, 70, 255, 246, 122, 70, 255, 246, 122, 70, 255, 246, 121, 70, 255, 245, 121, 70, 255, 245, 120, 69, 255, 245, 120, 69, 255, 245, 119, 69, 255, 245, 119, 69, 255, 245, 118, 69, 255, 245, 117, 69, 255, 245, 117, 68, 255, 245, 116, 68, 255, 245, 116, 68, 255, 245, 115, 68, 255, 245, 115, 68, 255, 244, 114, 68, 255, 244, 114, 68, 255, 244, 113, 67, 255, 244, 113, 67, 255, 244, 112, 67, 255, 244, 111, 67, 255, 244, 111, 67, 255, 244, 110, 67, 255, 244, 110, 67, 255, 244, 109, 67, 255, 244, 109, 67, 255, 243, 108, 67, 255, 243, 108, 67, 255, 243, 107, 67, 255, 243, 107, 67, 255, 243, 107, 67, 255, 242, 106, 67, 255, 242, 106, 67, 255, 242, 106, 67, 255, 242, 105, 67, 255, 242, 105, 67, 255, 241, 104, 68, 255, 241, 104, 68, 255, 241, 104, 68, 255, 241, 103, 68, 255, 241, 103, 68, 255, 240, 102, 68, 255, 240, 102, 68, 255, 240, 102, 68, 255, 240, 101, 68, 255, 240, 101, 68, 255, 239, 101, 69, 255, 239, 100, 69, 255, 239, 100, 69, 255, 239, 99, 69, 255, 238, 99, 69, 255, 238, 99, 69, 255, 238, 98, 69, 255, 238, 98, 69, 255, 238, 98, 69, 255, 237, 97, 69, 255, 237, 97, 70, 255, 237, 96, 70, 255, 237, 96, 70, 255, 236, 96, 70, 255, 236, 95, 70, 255, 236, 95, 70, 255, 236, 95, 70, 255, 236, 94, 70, 255, 235, 94, 70, 255, 235, 94, 70, 255, 235, 93, 71, 255, 235, 93, 71, 255, 234, 92, 71, 255, 234, 92, 71, 255, 234, 92, 71, 255, 234, 91, 71, 255, 233, 91, 71, 255, 233, 91, 71, 255, 233, 90, 71, 255, 233, 90, 71, 255, 233, 89, 72, 255, 226, 80, 74, 255, 226, 79, 74, 255, 226, 79, 74, 255, 225, 79, 74, 255, 225, 78, 74, 255, 225, 78, 74, 255, 225, 77, 75, 255, 224, 77, 75, 255, 224, 77, 75, 255, 224, 76, 75, 255, 224, 76, 75, 255, 223, 76, 75, 255, 223, 75, 75, 255, 223, 75, 75, 255, 223, 75, 75, 255, 222, 74, 75, 255, 222, 74, 75, 255, 222, 73, 76, 255, 222, 73, 76, 255, 221, 73, 76, 255, 221, 72, 76, 255, 221, 72, 76, 255, 220, 72, 76, 255, 220, 71, 76, 255, 220, 71, 76, 255, 220, 71, 76, 255, 219, 70, 76, 255, 219, 70, 76, 255, 219, 70, 77, 255, 219, 69, 77, 255, 218, 69, 77, 255, 218, 68, 77, 255, 218, 68, 77, 255, 217, 68, 77, 255, 217, 67, 77, 255, 217, 67, 77, 255, 217, 67, 77, 255, 216, 66, 77, 255, 216, 66, 77, 255, 216, 66, 78, 255, 216, 65, 78, 255, 215, 65, 78, 255, 215, 65, 78, 255, 215, 64, 78, 255, 214, 64, 78, 255, 214, 63, 78, 255, 214, 63, 78, 255, 214, 63, 78, 255, 213, 62, 78, 255, 213, 62, 78, 255, 213, 62, 78, 255, 212, 61, 78, 255, 212, 61, 78, 255, 211, 61, 78, 255, 211, 60, 78, 255, 211, 60, 78, 255, 210, 59, 78, 255, 210, 59, 78, 255, 209, 59, 78, 255, 209, 58, 78, 255, 209, 58, 78, 255, 208, 57, 78, 255, 208, 57, 77, 255, 207, 57, 77, 255, 207, 56, 77, 255, 206, 56, 77, 255, 206, 56, 77, 255, 206, 55, 77, 255, 205, 55, 77, 255, 205, 54, 77, 255, 204, 54, 77, 255, 204, 54, 77, 255, 203, 53, 77, 255, 203, 53, 76, 255, 203, 52, 76, 255, 202, 52, 76, 255, 202, 52, 76, 255, 201, 51, 76, 255, 201, 51, 76, 255, 200, 50, 76, 255, 200, 50, 76, 255, 200, 50, 76, 255, 199, 49, 76, 255, 199, 49, 76, 255, 198, 48, 75, 255, 198, 48, 75, 255, 198, 48, 75, 255, 197, 47, 75, 255, 197, 47, 75, 255, 196, 46, 75, 255, 196, 46, 75, 255, 195, 46, 75, 255, 195, 45, 75, 255, 195, 45, 75, 255, 194, 44, 74, 255, 194, 44, 74, 255, 193, 43, 74, 255, 193, 43, 74, 255, 192, 43, 74, 255, 192, 42, 74, 255, 192, 42, 74, 255, 191, 41, 74, 255, 180, 29, 71, 255, 179, 28, 71, 255, 179, 28, 71, 255, 178, 27, 71, 255, 178, 27, 71, 255, 177, 27, 71, 255, 177, 26, 70, 255, 177, 26, 70, 255, 176, 25, 70, 255, 176, 25, 70, 255, 175, 24, 70, 255, 175, 24, 70, 255, 174, 23, 70, 255, 174, 23, 70, 255, 174, 23, 70, 255, 173, 22, 70, 255, 173, 22, 69, 255, 172, 21, 69, 255, 172, 21, 69, 255, 171, 20, 69, 255, 171, 20, 69, 255, 171, 19, 69, 255, 170, 19, 69, 255, 170, 18, 69, 255, 169, 18, 69, 255, 169, 17, 68, 255, 168, 17, 68, 255, 168, 16, 68, 255, 168, 16, 68, 255, 167, 15, 68, 255, 167, 14, 68, 255, 166, 14, 68, 255, 166, 13, 68, 255, 165, 13, 68, 255, 165, 12, 67, 255, 164, 12, 67, 255, 164, 11, 67, 255, 164, 10, 67, 255, 163, 10, 67, 255, 163, 9, 67, 255, 162, 8, 67, 255, 162, 7, 67, 255, 161, 7, 67, 255, 161, 6, 66, 255, 161, 5, 66, 255, 160, 5, 66, 255, 160, 4, 66, 255, 159, 3, 66, 255, 159, 2, 66, 255, 158, 2, 66, 255, 158, 1, 66, 255
};

heatmap_writer_t::heatmap_writer_t()
    : skip(60), bounds(std::make_pair(0.02, 0.98))
{}

constexpr unsigned char *get_color(double val) {
    return mixed_data + ((int) (val * (sizeof(mixed_data) / (sizeof(mixed_data[0])*4) - 1) + 0.5f)*4);
}

std::tuple<float, float> wotreplay::get_bounds(boost::multi_array<float, 3>::const_reference image, float l_quant,float r_quant) {
    std::vector<float> values;
    auto not_zero = [](float value) { return value != 0.f; };
    std::copy_if(image.origin(), image.origin() + image.num_elements(), std::inserter(values, values.begin()), not_zero);

    if (values.size() == 0) {
        return std::make_tuple(0, std::numeric_limits<float>::max());
    }

    int l = std::lround(l_quant*(values.size() - 1)),
        u = std::lround(r_quant*(values.size() - 1));
    std::nth_element(values.begin(), values.begin() + l, values.end());
    float l_value = values[l];
    std::nth_element(values.begin(), values.begin() + u, values.end());
    float u_value = values[u];
    return std::make_tuple(l_value, u_value);
}

void heatmap_writer_t::finish() {
    load_base_map(arena.mini_map);
    const size_t *shape = base.shape();
    result.resize(boost::extents[shape[0]][shape[1]][shape[2]]);
    draw_elements();
    result = base;

    if (mode == heatmap_mode_t::combined) {
        for (int y = 0; y < shape[0]; y += 1) {
            for (int x = 0; x < shape[1]; x += 1) {
                float val = positions[1][y][x] + positions[0][y][x];
                for (int i = 0; i < 9; i += 1) {
                    for (int j = 0; j < 9; j += 1) {
                        if ((y + i - 5) >= 0 && (y + i - 5) < shape[0]
                            && (x + j - 5) >= 0 && (x + j - 5) < shape[1]) {
                            positions[2][y + i - 5][x + j - 5] += val*stamp_default_4_data[i*9+j];
                        }
                    }
                }
            }
        }

        double min, max;
        std::tie(min, max) = get_bounds(positions[2],
                                        std::get<0>(bounds),
                                        std::get<1>(bounds));

        for (int i = 0; i < shape[0]; i += 1) {
            for (int j = 0; j < shape[1]; j += 1) {
                double val = clamp(positions[2][i][j], min, max);
                const unsigned char *c = get_color((val - min) / (max - min));
                double a = c[3] * .66 / 255.0;
                result[i][j][0] = mix(result[i][j][0], result[i][j][0], 1 - a, c[0], a);
                result[i][j][1] = mix(result[i][j][1], result[i][j][1], 1 - a, c[1], a);
                result[i][j][2] = mix(result[i][j][2], result[i][j][2], 1 - a, c[2], a);
            }
        }
    } else {
        double min[2], max[2];

        for (int k = 0; k < 2; k += 1) {
            if (mode == heatmap_mode_t::team_soft) {
                std::unique_ptr<float[]> result(new float[image_width*image_height]());

                for (int y = 0; y < shape[0]; y += 1) {
                    for (int x = 0; x < shape[1]; x += 1) {
                        float val = positions[k][y][x];
                        for (int i = 0; i < 9; i += 1) {
                            for (int j = 0; j < 9; j += 1) {
                                if ((y + i - 5) >= 0 && (y + i - 5) < image_height &&
                                    (x + j - 5) >= 0 && (x + j - 5) < image_width) {
                                    result[(y + i - 5)*image_width + (x + j - 5)] += val*stamp_default_4_data[i*9+j];
                                }
                            }
                        }
                    }
                }

                std::copy_n(result.get(), image_width*image_height, positions[k].origin());
            }

            std::tie(min[k], max[k]) = get_bounds(positions[k],
                                                  std::get<0>(bounds),
                                                  std::get<1>(bounds));
        }

        static auto f =  mode == team_soft ? [](double x) {
            x = clamp(x, 0.0, 1.0);
            auto y = log10(0.99*x+0.01)/2. + 1.;
            return clamp(y, 0.0, 1.0);
        } : [](double x) { return x; };

        for (int i = 0; i < shape[0]; i += 1) {
            for (int j = 0; j < shape[1]; j += 1) {
                double a[] = {
                    clamp((positions[0][i][j] - min[0]) / (max[0] - min[0]), 0.0, 1.0),
                    clamp((positions[1][i][j] - min[1]) / (max[1] - min[1]), 0.0, 1.0)
                };

                a[0] = f(a[0]);
                a[1] = f(a[1]);

                result[i][j][0] = mix(result[i][j][0], 0, a[0], 255, a[1]);
                result[i][j][1] = mix(result[i][j][1], 255, a[0], 0, a[1]);
                result[i][j][2] = mix(result[i][j][2], 0, a[0], 0, a[1]);
            }
        }
    }
}

void heatmap_writer_t::update(const game_t &game) {
    std::set<int> dead_players;

    const auto &packets = game.get_packets();
    int i = 0, offset = get_start_packet(game, skip);
    for (auto it = packets.begin(); it != packets.end() ; it++ ) {
        const auto &packet = *it;
        ++i;
        if (!packet.has_property(property_t::position)) {
            if (packet.has_property(property_t::tank_destroyed)) {
                uint32_t target, killer;
                std::tie(target, killer) = packet.tank_destroyed();
                dead_players.insert(target);
            }
            continue;
        }

        if (i < offset) {
            continue;
        }

        uint32_t player_id = packet.player_id();
        int team_id = game.get_team_id(player_id);
        if (team_id < 0 || dead_players.find(player_id) != dead_players.end()) {
            continue;
        }

        auto shape = positions.shape();
        int height = static_cast<int>(shape[1]);
        int width = static_cast<int>(shape[2]);

        const bounding_box_t &bounding_box = game.get_arena().bounding_box;
        std::tuple<float, float> position = get_2d_coord( packet.position(), bounding_box, width, height);
        double x = std::get<0>(position);
        double y = std::get<1>(position);

        if (x >= 0 && y >= 0 && x <= (width - 1) && y <= (height - 1)) {
            float px = x - floor(x), py = y - floor(y);
            positions[team_id][floor(y)][floor(x)] += px*py;
            positions[team_id][ceil(y)][floor(x)] += px*(1-py);
            positions[team_id][floor(y)][ceil(x)] += (1-px)*py;
            positions[team_id][ceil(y)][ceil(x)] += (1-px)*(1-py);
        }
    }
}