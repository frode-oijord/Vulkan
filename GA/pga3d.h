
#include <array>
#include <string>
#include <vector>
#include <numbers>

typedef std::array<float, 16> PGA3D;

static const std::string basis[]{
	"1", "e0", "e1", "e2", "e3", "e01", "e02", "e03", "e12", "e31", "e23", "e021", "e013", "e032", "e123", "e0123"
};

std::string to_string(const PGA3D& b) {
	std::vector<std::pair<std::string, float>> values;

	for (size_t i = 0; i < 16; i++) {
		if (b[i] != 0.0f) {
			values.push_back({ basis[i], b[i] });
		}
	}

	if (values.empty()) {
		return "0";
	}

	std::string s;
	for (size_t i = 0; i < values.size(); i++) {
		float value = values[i].second;
		if (value == -1.0f) {
			s += "-";
		}
		else if (value != 1.0f) {
			s += std::to_string(value) + " * ";
		}
		s += values[i].first;
		if (i < values.size() - 1) {
			s += " + ";
		}
	}
	return s;
}

// Reverse the order of the basis blades.
PGA3D operator ~ (PGA3D a) {
	for (size_t i = 5; i < 16; i++) {
		a[i] = -a[i];
	}
	return a;
};

// Poincare duality operator.
PGA3D operator ! (PGA3D a) {
	std::reverse(a.begin(), a.end());
	return a;
};

// Clifford Conjugation
PGA3D conjugate(PGA3D a) {
	for (size_t i = 1; i < 11; i++) {
		a[i] = -a[i];
	}
	return a;
};

// Main involution
PGA3D involute(PGA3D a) {
	for (size_t i = 1; i < 5; i++) {
		a[i] = -a[i];
	}
	for (size_t i = 11; i < 15; i++) {
		a[i] = -a[i];
	}
	return a;
};

// The geometric product.
PGA3D operator * (const PGA3D& a, const PGA3D& b) {
	return {
		b[0] * a[0] + b[2] * a[2] + b[3] * a[3] + b[4] * a[4] - b[8] * a[8] - b[9] * a[9] - b[10] * a[10] - b[14] * a[14],
		b[1] * a[0] + b[0] * a[1] - b[5] * a[2] - b[6] * a[3] - b[7] * a[4] + b[2] * a[5] + b[3] * a[6] + b[4] * a[7] + b[11] * a[8] + b[12] * a[9] + b[13] * a[10] + b[8] * a[11] + b[9] * a[12] + b[10] * a[13] + b[15] * a[14] - b[14] * a[15],
		b[2] * a[0] + b[0] * a[2] - b[8] * a[3] + b[9] * a[4] + b[3] * a[8] - b[4] * a[9] - b[14] * a[10] - b[10] * a[14],
		b[3] * a[0] + b[8] * a[2] + b[0] * a[3] - b[10] * a[4] - b[2] * a[8] - b[14] * a[9] + b[4] * a[10] - b[9] * a[14],
		b[4] * a[0] - b[9] * a[2] + b[10] * a[3] + b[0] * a[4] - b[14] * a[8] + b[2] * a[9] - b[3] * a[10] - b[8] * a[14],
		b[5] * a[0] + b[2] * a[1] - b[1] * a[2] - b[11] * a[3] + b[12] * a[4] + b[0] * a[5] - b[8] * a[6] + b[9] * a[7] + b[6] * a[8] - b[7] * a[9] - b[15] * a[10] - b[3] * a[11] + b[4] * a[12] + b[14] * a[13] - b[13] * a[14] - b[10] * a[15],
		b[6] * a[0] + b[3] * a[1] + b[11] * a[2] - b[1] * a[3] - b[13] * a[4] + b[8] * a[5] + b[0] * a[6] - b[10] * a[7] - b[5] * a[8] - b[15] * a[9] + b[7] * a[10] + b[2] * a[11] + b[14] * a[12] - b[4] * a[13] - b[12] * a[14] - b[9] * a[15],
		b[7] * a[0] + b[4] * a[1] - b[12] * a[2] + b[13] * a[3] - b[1] * a[4] - b[9] * a[5] + b[10] * a[6] + b[0] * a[7] - b[15] * a[8] + b[5] * a[9] - b[6] * a[10] + b[14] * a[11] - b[2] * a[12] + b[3] * a[13] - b[11] * a[14] - b[8] * a[15],
		b[8] * a[0] + b[3] * a[2] - b[2] * a[3] + b[14] * a[4] + b[0] * a[8] + b[10] * a[9] - b[9] * a[10] + b[4] * a[14],
		b[9] * a[0] - b[4] * a[2] + b[14] * a[3] + b[2] * a[4] - b[10] * a[8] + b[0] * a[9] + b[8] * a[10] + b[3] * a[14],
		b[10] * a[0] + b[14] * a[2] + b[4] * a[3] - b[3] * a[4] + b[9] * a[8] - b[8] * a[9] + b[0] * a[10] + b[2] * a[14],
		b[11] * a[0] - b[8] * a[1] + b[6] * a[2] - b[5] * a[3] + b[15] * a[4] - b[3] * a[5] + b[2] * a[6] - b[14] * a[7] - b[1] * a[8] + b[13] * a[9] - b[12] * a[10] + b[0] * a[11] + b[10] * a[12] - b[9] * a[13] + b[7] * a[14] - b[4] * a[15],
		b[12] * a[0] - b[9] * a[1] - b[7] * a[2] + b[15] * a[3] + b[5] * a[4] + b[4] * a[5] - b[14] * a[6] - b[2] * a[7] - b[13] * a[8] - b[1] * a[9] + b[11] * a[10] - b[10] * a[11] + b[0] * a[12] + b[8] * a[13] + b[6] * a[14] - b[3] * a[15],
		b[13] * a[0] - b[10] * a[1] + b[15] * a[2] + b[7] * a[3] - b[6] * a[4] - b[14] * a[5] - b[4] * a[6] + b[3] * a[7] + b[12] * a[8] - b[11] * a[9] - b[1] * a[10] + b[9] * a[11] - b[8] * a[12] + b[0] * a[13] + b[5] * a[14] - b[2] * a[15],
		b[14] * a[0] + b[10] * a[2] + b[9] * a[3] + b[8] * a[4] + b[4] * a[8] + b[3] * a[9] + b[2] * a[10] + b[0] * a[14],
		b[15] * a[0] + b[14] * a[1] + b[13] * a[2] + b[12] * a[3] + b[11] * a[4] + b[10] * a[5] + b[9] * a[6] + b[8] * a[7] + b[7] * a[8] + b[6] * a[9] + b[5] * a[10] - b[4] * a[11] - b[3] * a[12] - b[2] * a[13] - b[1] * a[14] + b[0] * a[15],
	};
};

// The outer product. (MEET)
constexpr PGA3D operator ^ (const PGA3D& a, const PGA3D& b) {
	return {
		b[0] * a[0],
		b[1] * a[0] + b[0] * a[1],
		b[2] * a[0] + b[0] * a[2],
		b[3] * a[0] + b[0] * a[3],
		b[4] * a[0] + b[0] * a[4],
		b[5] * a[0] + b[2] * a[1] - b[1] * a[2] + b[0] * a[5],
		b[6] * a[0] + b[3] * a[1] - b[1] * a[3] + b[0] * a[6],
		b[7] * a[0] + b[4] * a[1] - b[1] * a[4] + b[0] * a[7],
		b[8] * a[0] + b[3] * a[2] - b[2] * a[3] + b[0] * a[8],
		b[9] * a[0] - b[4] * a[2] + b[2] * a[4] + b[0] * a[9],
		b[10] * a[0] + b[4] * a[3] - b[3] * a[4] + b[0] * a[10],
		b[11] * a[0] - b[8] * a[1] + b[6] * a[2] - b[5] * a[3] - b[3] * a[5] + b[2] * a[6] - b[1] * a[8] + b[0] * a[11],
		b[12] * a[0] - b[9] * a[1] - b[7] * a[2] + b[5] * a[4] + b[4] * a[5] - b[2] * a[7] - b[1] * a[9] + b[0] * a[12],
		b[13] * a[0] - b[10] * a[1] + b[7] * a[3] - b[6] * a[4] - b[4] * a[6] + b[3] * a[7] - b[1] * a[10] + b[0] * a[13],
		b[14] * a[0] + b[10] * a[2] + b[9] * a[3] + b[8] * a[4] + b[4] * a[8] + b[3] * a[9] + b[2] * a[10] + b[0] * a[14],
		b[15] * a[0] + b[14] * a[1] + b[13] * a[2] + b[12] * a[3] + b[11] * a[4] + b[10] * a[5] + b[9] * a[6] + b[8] * a[7] + b[7] * a[8] + b[6] * a[9] + b[5] * a[10] - b[4] * a[11] - b[3] * a[12] - b[2] * a[13] - b[1] * a[14] + b[0] * a[15],
	};
};

// The regressive product. (JOIN)
PGA3D operator & (const PGA3D& a, const PGA3D& b) {
	return {
		1 * (a[0] * b[15] + a[1] * b[14] * -1 + a[2] * b[13] * -1 + a[3] * b[12] * -1 + a[4] * b[11] * -1 + a[5] * b[10] + a[6] * b[9] + a[7] * b[8] + a[8] * b[7] + a[9] * b[6] + a[10] * b[5] - a[11] * -1 * b[4] - a[12] * -1 * b[3] - a[13] * -1 * b[2] - a[14] * -1 * b[1] + a[15] * b[0]),
		1 * (a[1] * b[15] + a[5] * b[13] * -1 + a[6] * b[12] * -1 + a[7] * b[11] * -1 + a[11] * -1 * b[7] + a[12] * -1 * b[6] + a[13] * -1 * b[5] + a[15] * b[1]),
		1 * (a[2] * b[15] - a[5] * b[14] * -1 + a[8] * b[12] * -1 - a[9] * b[11] * -1 - a[11] * -1 * b[9] + a[12] * -1 * b[8] - a[14] * -1 * b[5] + a[15] * b[2]),
		1 * (a[3] * b[15] - a[6] * b[14] * -1 - a[8] * b[13] * -1 + a[10] * b[11] * -1 + a[11] * -1 * b[10] - a[13] * -1 * b[8] - a[14] * -1 * b[6] + a[15] * b[3]),
		1 * (a[4] * b[15] - a[7] * b[14] * -1 + a[9] * b[13] * -1 - a[10] * b[12] * -1 - a[12] * -1 * b[10] + a[13] * -1 * b[9] - a[14] * -1 * b[7] + a[15] * b[4]),
		1 * (a[5] * b[15] + a[11] * -1 * b[12] * -1 - a[12] * -1 * b[11] * -1 + a[15] * b[5]),
		1 * (a[6] * b[15] - a[11] * -1 * b[13] * -1 + a[13] * -1 * b[11] * -1 + a[15] * b[6]),
		1 * (a[7] * b[15] + a[12] * -1 * b[13] * -1 - a[13] * -1 * b[12] * -1 + a[15] * b[7]),
		1 * (a[8] * b[15] + a[11] * -1 * b[14] * -1 - a[14] * -1 * b[11] * -1 + a[15] * b[8]),
		1 * (a[9] * b[15] + a[12] * -1 * b[14] * -1 - a[14] * -1 * b[12] * -1 + a[15] * b[9]),
		1 * (a[10] * b[15] + a[13] * -1 * b[14] * -1 - a[14] * -1 * b[13] * -1 + a[15] * b[10]),
		-1 * (a[11] * -1 * b[15] + a[15] * b[11] * -1),
		-1 * (a[12] * -1 * b[15] + a[15] * b[12] * -1),
		-1 * (a[13] * -1 * b[15] + a[15] * b[13] * -1),
		-1 * (a[14] * -1 * b[15] + a[15] * b[14] * -1),
		1 * (a[15] * b[15]),
	};
};

// The inner product
PGA3D operator | (const PGA3D& a, const PGA3D& b) {
	return {
		b[0] * a[0] + b[2] * a[2] + b[3] * a[3] + b[4] * a[4] - b[8] * a[8] - b[9] * a[9] - b[10] * a[10] - b[14] * a[14],
		b[1] * a[0] + b[0] * a[1] - b[5] * a[2] - b[6] * a[3] - b[7] * a[4] + b[2] * a[5] + b[3] * a[6] + b[4] * a[7] + b[11] * a[8] + b[12] * a[9] + b[13] * a[10] + b[8] * a[11] + b[9] * a[12] + b[10] * a[13] + b[15] * a[14] - b[14] * a[15],
		b[2] * a[0] + b[0] * a[2] - b[8] * a[3] + b[9] * a[4] + b[3] * a[8] - b[4] * a[9] - b[14] * a[10] - b[10] * a[14],
		b[3] * a[0] + b[8] * a[2] + b[0] * a[3] - b[10] * a[4] - b[2] * a[8] - b[14] * a[9] + b[4] * a[10] - b[9] * a[14],
		b[4] * a[0] - b[9] * a[2] + b[10] * a[3] + b[0] * a[4] - b[14] * a[8] + b[2] * a[9] - b[3] * a[10] - b[8] * a[14],
		b[5] * a[0] - b[11] * a[3] + b[12] * a[4] + b[0] * a[5] - b[15] * a[10] - b[3] * a[11] + b[4] * a[12] - b[10] * a[15],
		b[6] * a[0] + b[11] * a[2] - b[13] * a[4] + b[0] * a[6] - b[15] * a[9] + b[2] * a[11] - b[4] * a[13] - b[9] * a[15],
		b[7] * a[0] - b[12] * a[2] + b[13] * a[3] + b[0] * a[7] - b[15] * a[8] - b[2] * a[12] + b[3] * a[13] - b[8] * a[15],
		b[8] * a[0] + b[14] * a[4] + b[0] * a[8] + b[4] * a[14],
		b[9] * a[0] + b[14] * a[3] + b[0] * a[9] + b[3] * a[14],
		b[10] * a[0] + b[14] * a[2] + b[0] * a[10] + b[2] * a[14],
		b[11] * a[0] + b[15] * a[4] + b[0] * a[11] - b[4] * a[15],
		b[12] * a[0] + b[15] * a[3] + b[0] * a[12] - b[3] * a[15],
		b[13] * a[0] + b[15] * a[2] + b[0] * a[13] - b[2] * a[15],
		b[14] * a[0] + b[0] * a[14],
		b[15] * a[0] + b[0] * a[15],
	};
};

PGA3D operator + (PGA3D a, const PGA3D& b) {
	for (size_t i = 0; i < 16; i++) {
		a[i] += b[i];
	}
	return a;
};

PGA3D operator - (PGA3D a, const PGA3D& b) {
	for (size_t i = 0; i < 16; i++) {
		a[i] -= b[i];
	}
	return a;
};

PGA3D operator * (const float a, PGA3D b) {
	for (size_t i = 0; i < 16; i++) {
		b[i] *= a;
	}
	return b;
};

PGA3D operator * (PGA3D b, const float a) {
	return a * b;
};

PGA3D operator + (const float a, PGA3D b) {
	b[0] += a;
	return b;
};

PGA3D operator + (PGA3D b, const float a) {
	return a + b;
};

PGA3D operator - (const float a, PGA3D b) {
	b[0] = a - b[0];
	for (size_t i = 1; i < 16; i++) {
		b[i] = -b[i];
	}
	return b;
};

PGA3D operator - (PGA3D a, const float b) {
	a[0] -= b;
	return a;
};

float norm(const PGA3D& b) {
	return sqrt(std::abs((b * conjugate(b))[0]));
}

float inorm(const PGA3D& b) {
	return norm(!b);
}

PGA3D normalize(const PGA3D& b) {
	return b * (1 / norm(b));
}


// A rotor (Euclidean line)
PGA3D rotor(float angle, const PGA3D& line) {
	return cos(angle / 2) + sin(angle / 2) * normalize(line);
}

// A translator (Ideal line)
PGA3D translator(float dist, const PGA3D& line) {
	return 1 + dist / 2 * line;
}

// PGA is plane based. Vectors are planes. (think linear functionals)
static constexpr PGA3D e0 = { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static constexpr PGA3D e1 = { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static constexpr PGA3D e2 = { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static constexpr PGA3D e3 = { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// PGA points are trivectors.
static constexpr PGA3D e123 = e1 ^ e2 ^ e3;
static constexpr PGA3D e032 = e0 ^ e3 ^ e2;
static constexpr PGA3D e013 = e0 ^ e1 ^ e3;
static constexpr PGA3D e021 = e0 ^ e2 ^ e1;

// A point is just a homogeneous point, euclidean coordinates plus the origin
PGA3D point(float x, float y, float z) {
	return e123 + x * e032 + y * e013 + z * e021;
}

// Lines
PGA3D line_from_points(const PGA3D& p1, const PGA3D& p2) {
	return p1 & p2;
}

PGA3D line_from_point_and_dir(const PGA3D& p, const PGA3D& d) {
	return p & (p + d);
}

// Planes
PGA3D plane_from_equation(float a, float b, float c, float d) {
	return a * e1 + b * e2 + c * e3 + d * e0;
}

PGA3D plane_from_points(const PGA3D& p1, const PGA3D& p2, const PGA3D& p3) {
	return p1 & p2 & p3;
}

PGA3D plane_from_points_and_dir(const PGA3D& p1, const PGA3D& p2, const PGA3D& d) {
	return p1 & p2 & (p1 + d);
}

PGA3D plane_from_point_and_dirs(const PGA3D& p, const PGA3D& d1, const PGA3D& d2) {
	return p & (p + d1) & (p + d2);
}

PGA3D plane_from_point_and_line(const PGA3D& p, const PGA3D& l) {
	return p & l;
}
