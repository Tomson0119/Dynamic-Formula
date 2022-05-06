#pragma once

#include <string_view>

class BtShapeInterface
{
public:
	virtual ~BtShapeInterface() = default;
	virtual void LoadShapeData(std::string_view filename) = 0;

	const btVector3& GetExtents() const { return mExtents; }

protected:
	btVector3 mExtents;
};