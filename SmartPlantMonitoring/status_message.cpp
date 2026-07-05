#include "status_message.h"
#include "globals.h"

String buildStatusMessage() {
  String msg = "Motion detected! ";

  msg += "T=";
  if (isnan(lastT)) msg += "NA";
  else msg += String(lastT, 1);
  msg += "C, H=";

  if (isnan(lastH)) msg += "NA";
  else msg += String(lastH, 0);
  msg += "%, Soil=";

  if (lastSoilPct < 0) msg += "NA";
  else msg += String(lastSoilPct);
  msg += "%";

  return msg;
}
