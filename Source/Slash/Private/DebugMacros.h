#pragma once
#include "DrawDebugHelpers.h"

#define DRAW_SPHERE(Location, Color) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 24, Color, true);
#define DRAW_SPHERE_SINGLEFRAME(Location, Color) if(GetWorld()) DrawDebugSphere(GetWorld(), Location, 25.f, 24, Color, false, -1.f);
#define DRAW_LINE(StartLocation, EndLocation) if(GetWorld()) DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, true);
#define DRAW_LINE_SINGLEFRAME(StartLocation, EndLocation) if(GetWorld()) DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, -1.f);
#define DRAW_POINT(Location) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, 25.f, FColor::Red, true);
#define DRAW_POINT_SINGLEFRAME(Location) if(GetWorld()) DrawDebugPoint(GetWorld(), Location, 25.f, FColor::Red, false, -1.f);
#define DRAW_CUBE(Location, Scale) if(GetWorld()) DrawDebugBox(GetWorld(), Location, FVector(Scale), FColor::Yellow, true, 30);
#define DRAW_CUBE_SINGLEFRAME(Location, Scale) if(GetWorld()) DrawDebugBox(GetWorld(), Location, FVector(Scale), FColor::Yellow, false, -1.f);
#define DRAW_VECTOR(StartLocation, EndLocation) if(GetWorld()) \
	{ \
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 60);\
	DrawDebugPoint(GetWorld(), EndLocation, 25.f, FColor::Red, true, 30);\
	}
#define DRAW_VECTOR_SINGLEFRAME(StartLocation, EndLocation) if(GetWorld()) \
	{ \
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, -1.f);\
	DrawDebugPoint(GetWorld(), EndLocation, 25.f, FColor::Red, false, -1.f);\
	}
