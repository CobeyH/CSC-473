#pragma once
#include "BaseSystem.h"
#include "BodyPart.h"
#include <vector>

#define UPPER_ARM_RATIO 1.5
#define LOWER_ARM_RATIO 1.5
#define TORSO_RATIO 2.5
#define LEG_RATIO 1.3
#define HAND_RATIO 0.5
#define HEAD_RATIO 0.75

enum Orientation {
	Left,
	Right
};

class HumanModel : public BaseSystem {
public:
	HumanModel(const std::string& name);
	void getState(double* p);
	void setState(double* p);
	void reset(double time);
	void display(GLenum mode = GL_RENDER);
	int command(int argc, myCONST_SPEC char** argv);
	BodyPart* root;
private:
	void createBody();
	void createArm(Orientation side);
	void createLeg(Orientation side);
	void computeJacobian();
	double* angles[7];
};

struct Jacobian {
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	Eigen::Matrix<double, 7, 3> jacobian;
	Eigen::Matrix<double, 4, 4> Troot, Tsh, R1, R2, R3, Tel, R4, R5, Twr, R6, R7;
	HumanModel* body;
	BodyPart *shoulder, *elbow, *wrist;

	Jacobian(HumanModel* bodyState) {
		body = bodyState;
		shoulder = body->root->getChild(1);
		elbow = shoulder->getChild(0);
		wrist = elbow->getChild(0);

		Troot = body->root->getTranslationMatrix();
		Tsh = shoulder->getTranslationMatrix();
		R1 = shoulder->getZRotMatrix();
		R2 = shoulder->getYRotMatrix();
		R3 = shoulder->getXRotMatrix();
		Tel = elbow->getTranslationMatrix();
		R4 = elbow->getYRotMatrix();
		R5 = elbow->getXRotMatrix();
		Twr = wrist->getTranslationMatrix();
		R6 = wrist->getYRotMatrix();
		R7 = wrist->getZRotMatrix();
	}

	void computeJacobian() {
		for (int i = 1; i < 8; i++) {
			Eigen::Vector3d nextCol = computeColumn(i);
			jacobian(i - 1, 0) = nextCol[0];
			jacobian(i - 1, 1) = nextCol[1];
			jacobian(i - 1, 2) = nextCol[2];
		}
	}

	Eigen::Vector3d computeColumn(int i) {
		Eigen::Vector4d phand = {0, wrist->getHeight() / 2, 0, 1};
		Eigen::Vector3d result;
		Eigen::Vector<double, 4> column = Troot * Tsh *
			(i != 3 ? R1 : shoulder->getZRotMatrixDer()) *
			(i != 2 ? R2 : shoulder->getYRotMatrixDer()) *
			(i != 1 ? R3 : shoulder->getXRotMatrixDer()) *
			Tel *
			(i != 5 ? R4 : elbow->getYRotMatrixDer()) *
			(i != 4 ? R5 : elbow->getXRotMatrixDer()) *
			Twr *
			(i != 6 ? R6 : wrist->getYRotMatrixDer()) *
			(i != 7 ? R7 : wrist->getZRotMatrixDer()) 
		* phand;
		result[0] = column[0];
		result[1] = column[1];
		result[2] = column[2];
		return result;
	}
};