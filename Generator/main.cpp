
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <glm/glm.hpp>
#include <PerlinNoise.hpp>



int main(int argc, char* argv[])
{
	size_t lod0_size = 256;
	size_t num_lods = log2(lod0_size) + 1;

	const siv::PerlinNoise perlin;
	const int octaves = 8;
	const double dx = lod0_size / octaves;

	std::string filename = "noise" + std::to_string(lod0_size) + ".bin";

	//std::fstream out(filename, std::ios_base::out | std::ios_base::binary);
	//for (size_t lod = 0; lod < num_lods; lod++) {
	//	size_t lod_size = lod0_size >> lod;
	//	for (size_t start_i = 0; start_i < lod_size; start_i += 32) {
	//		std::cout << "i = " << start_i << std::endl;
	//		for (size_t start_j = 0; start_j < lod_size; start_j += 32) {
	//			for (size_t start_k = 0; start_k < lod_size; start_k += 16) {
	//				std::vector<glm::u8vec4> brick;
	//				for (size_t i = 0; i < 32; i++) {
	//						for (size_t k = 0; k < 16; k++) {
	//					for (size_t j = 0; j < 32; j++) {
	//							//glm::u8vec4 texel((i / 31.0) * 255.0, (j / 31.0) * 255.0, (k / 15.0) * 255.0, 255);
	//							glm::u8vec4 texel(i + start_i, j + start_j, k + start_k, 255);
	//							brick.push_back(texel);
	//							//double value = perlin.accumulatedOctaveNoise3D_0_1(x / dx, y / dx, z / dx, octaves);
	//							//glm::u8vec4 texel(value * 255.0);
	//							//brick[i + j * 32 + k * 32 * 32] = texel;
	//						}
	//					}
	//				}
	//				out.write(reinterpret_cast<char*>(brick.data()), brick.size() * 4);
	//			}
	//		}
	//	}
	//}

	std::vector<glm::u8vec4> data;
	if (!std::filesystem::exists(filename)) {
		std::fstream out(filename, std::ios_base::out | std::ios_base::binary);
		for (size_t lod = 0; lod < num_lods; lod++) {
			size_t lod_size = lod0_size >> lod;
			for (size_t i = 0; i < lod_size; i++) {
				std::cout << "i = " << i << std::endl;
				for (size_t j = 0; j < lod_size; j++) {
					for (size_t k = 0; k < lod_size; k++) {
						double value = perlin.accumulatedOctaveNoise3D_0_1(i / dx, j / dx, k / dx, octaves);
						data.push_back(glm::u8vec4(value * 255.0));
					}
				}
			}
		}
		out.write(reinterpret_cast<char*>(data.data()), data.size() * 4);
	}

	return 1;
}
