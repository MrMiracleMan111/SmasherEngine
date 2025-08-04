#include "Base.h"

namespace Smasher {

	template<>
	void Matrix<3, 3>::copyMatrix(const sf::Transform& transform) {
		const float* tmp = transform.getMatrix();
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 3; j++) {
				array[i * 3 + j] = tmp[i * 4 + j];
			}
		}

		// Last Row is [0 0 1] to satify homogenous coordinates
		array[6] = 0.0f;
		array[7] = 0.0f;
		array[8] = 1.0f;
	}

	template<>
	void Matrix<4, 4>::copyMatrix(const sf::Transform& transform) {
		copyMatrix(transform.getMatrix(), 4 * 4, array);
	}

}
