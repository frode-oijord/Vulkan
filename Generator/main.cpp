
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <glm/glm.hpp>
#include <PerlinNoise.hpp>

int main(int argc, char* argv[])
{
	size_t lod0_size = 4096;
	size_t num_lods = log2(lod0_size) + 1;

	const siv::PerlinNoise perlin;
	const int octaves = 8;
	const double dx = lod0_size / octaves;

	std::string filename = "noise" + std::to_string(lod0_size) + ".bin";

	if (!std::filesystem::exists(filename)) {
		std::fstream out(filename, std::ios_base::out | std::ios_base::binary);
		for (size_t lod = 0; lod < num_lods; lod++) {
			size_t lod_size = lod0_size >> lod;
			glm::u8vec1* data = new glm::u8vec1[lod_size * lod_size * lod_size];
			for (size_t i = 0; i < lod_size; i++) {
				std::cout << "i = " << i << std::endl;
				for (size_t j = 0; j < lod_size; j++) {
					for (size_t k = 0; k < lod_size; k++) {
						double value = perlin.accumulatedOctaveNoise3D_0_1(i / dx, j / dx, k / dx, octaves);
						data[((i * lod_size) + j) * lod_size + k] = glm::u8vec1(value * 255.0);
					}
				}
			}
			out.write(reinterpret_cast<char*>(data), lod_size * lod_size * lod_size);
		}
	}

	return 1;
}
