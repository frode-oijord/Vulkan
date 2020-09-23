
#include <pga3d.h>

#include <string>
#include <numbers>
#include <iostream>
#include <functional>

#define GREEN(__text__) "\033[1;32m" + std::string(__text__) + "\033[0m"
#define RED(__text__) "\033[1;31m" + std::string(__text__) + "\033[0m"

typedef std::function<bool()> test_case;

std::vector<test_case> tests
{
	[] {
		std::string result = to_string(point(1, 0, 0));
		bool pass = result == "e032 + e123";
		std::cout << "point(1, 0, 0) => " << result << " " << (pass ? GREEN("(Pass)") : RED("(Fail)")) << std::endl;
		return pass;
	},
	[] {
		PGA3D p1 = point(1, 2, -2);
		PGA3D p2 = point(3, -2, 1);
		PGA3D p3 = point(5, 1, -4);

		std::string plane1 = to_string(plane_from_points(p1, p2, p3));
		std::string plane2 = to_string(plane_from_equation(-11, -16, -14, 15));
		bool pass = plane1 == plane2;
		std::cout << "plane == " << plane1 << " " << (pass ? GREEN("(Pass)") : RED("(Fail)")) << std::endl;

		return pass;
	},
	[] {
		PGA3D line = e123 & point(1, 0, 0);
		std::cout << "a line        : " << to_string(line) << std::endl;
		return to_string(line) == "-e23";
	},
	[] {
		PGA3D p = plane_from_equation(2, 0, 1, -3);
		std::cout << "a plane       : " << to_string(p) << std::endl;
		return to_string(p) == "-3.000000 * e0 + 2.000000 * e1 + e3";
	},
	[] {
		PGA3D rot = rotor(std::numbers::pi / 2, e1 * e2);
		std::cout << "a rotor       : " << to_string(rot) << std::endl;
		return to_string(rot) == "0.707107 * 1 + 0.707107 * e12";
	},
	[] {
		PGA3D line = e123 & point(1, 0, 0);
		PGA3D rot = rotor(std::numbers::pi / 2, e1 * e2);
		PGA3D rotated_line = rot * line * ~rot;

		std::cout << "rotated line  : " << to_string(rotated_line) << std::endl;
		return to_string(rotated_line) == "1.000000 * e31";
	},
	[] {
		PGA3D px = point(1, 0, 0);
		PGA3D rot = rotor(std::numbers::pi / 2, e1 * e2);
		PGA3D rotated_point = rot * px * ~rot;
		std::cout << "rotated point : " << to_string(rotated_point) << std::endl;
		return to_string(rotated_point) == "-1.000000 * e013 + 1.000000 * e123";
	},
	[] {
		PGA3D p = plane_from_equation(2, 0, 1, -3);
		PGA3D rot = rotor(std::numbers::pi / 2, e1 * e2);
		PGA3D rotated_plane = rot * p * ~rot;
		std::cout << "rotated plane : " << to_string(rotated_plane) << std::endl;
		return to_string(rotated_plane) == "-3.000000 * e0 + -2.000000 * e2 + 1.000000 * e3";
	},
	[] {
		PGA3D p = plane_from_equation(2, 0, 1, -3);
		PGA3D px = point(1, 0, 0);
		PGA3D point_on_plane = (p | px) * p;
		std::cout << "point on plane: " << to_string(normalize(point_on_plane)) << std::endl;
		return to_string(normalize(point_on_plane)) == "0.200000 * e021 + 1.400000 * e032 + e123";
	},
	[] {
		auto point_on_torus = [](float s, float t) {
			auto torus = [](float s, float t, float r1, PGA3D l1, float r2, PGA3D l2) {
				auto circle = [](float t, float radius, PGA3D line) {
					return rotor(t * 2.0f * std::numbers::pi, line) * translator(radius, e1 * e0);
				};
				return circle(s, r2, l2) * circle(t, r1, l1);
			};

			PGA3D to = torus(s, t, 0.25f, e1 * e2, 0.6f, e1 * e3);
			return to * e123 * ~to;
		};

		std::cout << "point on torus: " << to_string(point_on_torus(0, 0)) << std::endl;
		return to_string(point_on_torus(0, 0)) == "0.850000 * e032 + e123";
	},
	[] {
		std::cout << to_string(e0 - 1.0f) << std::endl;
		return to_string(e0 - 1.0f) == "-1 + e0";
	},
	[] {
		std::cout << to_string(1.0f - e0) << std::endl;
		return to_string(1.0f - e0) == "1 + -e0";
	}
};


int main(int, char* [])
{
	std::vector<bool> results;

	for (auto test : tests) {
		results.push_back(test());
	}

	size_t n_exec = results.size();
	size_t n_fail = std::count(results.begin(), results.end(), false);
	size_t n_pass = std::count(results.begin(), results.end(), true);

	std::cout << std::endl << GREEN(std::to_string(n_exec) + " tests executed. ");

	if (n_fail > 0) {
		std::cout << RED(std::to_string(n_fail) + " tests failed. ") << std::endl;
	}
	else {
		std::cout << GREEN("All tests passed.") << std::endl;
	}

	return 1;
}
