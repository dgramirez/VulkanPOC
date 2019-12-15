#ifndef TEST0_TRIANGLE_H
#define TEST0_TRIANGLE_H

#include "../Tests.h"

class Test0_Triangle : public Test {
public:
	Test0_Triangle();
	~Test0_Triangle();

	void Update(const float &_deltaTime) override;
	void Render() override;
	void RenderImGui() override;

	virtual VoidFuncPtr SendCreatePipeline();
	virtual VoidFuncPtr SendDestroyPipeline();
private:
	Test0_Triangle(const Test0_Triangle &);
};

#endif