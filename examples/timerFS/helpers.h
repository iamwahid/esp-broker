#pragma once

#include <WString.h>

String pakanStatus(long dist) {
  String status_pakan = "LOW";
  if (dist >= 15) {
    status_pakan = "EMPTY";
  } else if (dist < 15 && dist >= 12) {
    status_pakan = "LOW";
  } else if (dist < 12 && dist >= 7) {
    status_pakan = "MEDIUM";
  } else if (dist < 7) {
    status_pakan = "HIGH";
  }
  return status_pakan;
}