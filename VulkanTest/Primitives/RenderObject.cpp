#include "RenderObject.h"
#include "../RenderVisitor.h"

void RenderObject::Accept(RenderVisitor& renderVisitor) const
{
	renderVisitor.Visit(*this);
}
