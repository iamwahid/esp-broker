#pragma once

#include <WString.h>

long mapV(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


int pakanMap(int jarak) {
  int level = mapV(jarak, 17, 2, 0, 10);
  return max(0, level);
}

String pakanLevel(long dist) {
  String status_pakan = "LOW";
  int pMap = pakanMap(dist);
  // if (dist >= 15) {
  //   status_pakan = "EMPTY";
  // } else if (dist < 15 && dist >= 12) {
  //   status_pakan = "LOW";
  // } else if (dist < 12 && dist >= 7) {
  //   status_pakan = "MEDIUM";
  // } else if (dist < 7) {
  //   status_pakan = "HIGH";
  // }

  if (pMap == 0) {
    status_pakan = "EMPTY";
  } else if (pMap > 0 && pMap <= 3) {
    status_pakan = "LOW";
  } else if (pMap > 3 && pMap <= 6) {
    status_pakan = "MEDIUM";
  } else if (pMap > 6) {
    status_pakan = "HIGH";
  }

  return status_pakan;
}