// ATTENTION: This file is largely generated.
// If you are going to use a enum provided
// here it might be beneficial to check over the naming
// and fix inconsistencies before usage.

#pragma once

#include <cstdint>
#include <string_view>

#include <glaze/core/common.hpp>

namespace tfc::ec::devices::schneider::atv320 {

/// ACT enum ATV320
enum struct act_e : std::uint16_t {
  tab = 0,   ///< Not done ([Not Done] (TAB))
  pend = 1,  ///< Test is pending ([Pending] (PEND))
  prog = 2,  ///< Test in progress ([In Progress] (PROG))
  fail = 3,  ///< Error detected ([Error] (FAIL))
  done = 4,  ///< Autotuning Done ([Autotuning Done] (DONE))
};
[[nodiscard]] constexpr auto enum_desc(act_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case act_e::tab:
      return "[Not Done] (TAB), Not done";
    case act_e::pend:
      return "[Pending] (PEND), Test is pending";
    case act_e::prog:
      return "[In Progress] (PROG), Test in progress";
    case act_e::fail:
      return "[Error] (FAIL), Error detected";
    case act_e::done:
      return "[Autotuning Done] (DONE), Autotuning Done";
  }
  return "unknown";
}
constexpr auto format_as(act_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of action_e enum decleration
enum struct action_e : std::uint16_t {
  no = 0,   ///< No action ([No Action] (NO))
  yes = 1,  ///< Apply autotuning ([Apply Autotuning] (YES))
  clr = 2,  ///< Erase autotuning ([Erase Autotuning] (CLR))
};
[[nodiscard]] constexpr auto enum_desc(action_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case action_e::no:
      return "[No Action] (NO), No action";
    case action_e::yes:
      return "[Apply Autotuning] (YES), Apply autotuning";
    case action_e::clr:
      return "[Erase Autotuning] (CLR), Erase autotuning";
  }
  return "unknown";
}
constexpr auto format_as(action_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of adc_e enum decleration
enum struct adc_e : std::uint16_t {
  no = 0,   ///< No DC injection ([No] (NO))
  yes = 1,  ///< DC injection ([Yes] (YES))
  ct = 2,   ///< Continuous DC injection ([Continuous] (CT))
};
[[nodiscard]] constexpr auto enum_desc(adc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case adc_e::no:
      return "[No] (NO), No DC injection";
    case adc_e::yes:
      return "[Yes] (YES), DC injection";
    case adc_e::ct:
      return "[Continuous] (CT), Continuous DC injection";
  }
  return "unknown";
}
constexpr auto format_as(adc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of affl_e enum decleration
enum struct affl_e : std::uint16_t {
  no = 0,   ///<  ([None] (NO))
  ch1 = 1,  ///<  ([Channel 1] (CH1))
  ch2 = 2,  ///<  ([Channel 2] (CH2))
};
[[nodiscard]] constexpr auto enum_desc(affl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case affl_e::no:
      return "[None] (NO), ";
    case affl_e::ch1:
      return "[Channel 1] (CH1), ";
    case affl_e::ch2:
      return "[Channel 2] (CH2), ";
  }
  return "unknown";
}
constexpr auto format_as(affl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of aiol_e enum decleration
enum struct aiol_e : std::uint16_t {
  positive_only = 0,     ///< Positive only ([0 - 100%] (POS))
  positive_bipolar = 1,  ///< Positive and negative ([+/- 100%] (POSNEG))
};
[[nodiscard]] constexpr auto enum_desc(aiol_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case aiol_e::positive_only:
      return "[0 - 100%] (POS), Positive only";
    case aiol_e::positive_bipolar:
      return "[+/- 100%] (POSNEG), Positive and negative";
  }
  return "unknown";
}
constexpr auto format_as(aiol_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of aiot_e enum decleration
enum struct aiot_e : std::uint16_t {
  voltage = 1,            ///< Voltage ([Voltage] (10U))
  current = 2,            ///< Current ([Current] (0A))
  voltage_bipolar = 5,    ///< AI bipolar volts selected ([Voltage bipolar] (N10U))
  ptc_management = 7,     ///< PTC MANAGEMENT ([PTC MANAGEMENT] (PTC))
  kty = 8,                ///< KTY ([KTY] (KTY))
  pt1000 = 9,             ///< PT1000 ([PT1000] (1PT3))
  pt100 = 10,             ///< PT100 ([PT100] (1PT2))
  water_prob = 11,        ///< Water Prob ([Water Prob] (LEVEL))
  pt1000_3 = 12,          ///< 3 PT1000 ([PT1000_3] (3PT3))
  pt100_3 = 13,           ///< 3 PT100 ([PT100_3] (3PT2))
  pt1000_3_wires = 14,    ///< PT1000 in 3 wires ([PT1000 in 3 wires] (1PT33))
  pt100_3_wires = 15,     ///< PT100 in 3 wires ([PT100 in 3 wires] (1PT23))
  pt1000_3_wires_3 = 16,  ///< 3 PT1000 in 3 wires ([PT1000 3 in 3 wires] (3PT33))
  pt100_3_wires_3 = 17,   ///< 3 PT100 in 3 wires ([PT100 3 in 3 wires] (3PT23))
};

[[nodiscard]] constexpr auto enum_desc(aiot_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case aiot_e::voltage:
      return "[Voltage] (10U), ";
    case aiot_e::current:
      return "[Current] (0A), ";
    case aiot_e::voltage_bipolar:
      return "[Voltage +/-] (n10U), ";
    case aiot_e::ptc_management:
      return "[PTC MANAGEMENT] (PTC), PTC MANAGEMENT";
    case aiot_e::kty:
      return "[KTY] (KTY), KTY";
    case aiot_e::pt1000:
      return "[PT1000] (1PT3), PT1000";
    case aiot_e::pt100:
      return "[PT100] (1PT2), PT100";
    case aiot_e::water_prob:
      return "[Water Prob] (LEVEL), Water Prob";
    case aiot_e::pt1000_3:
      return "[3 PT1000] (3PT3), 3 PT1000";
    case aiot_e::pt100_3:
      return "[3 PT100] (3PT2), 3 PT100";
    case aiot_e::pt100_3_wires:
      return "[PT1000 in 3 wires] (1PT33), PT1000 in 3 wires";
    case aiot_e::pt1000_3_wires:
      return "[PT100 in 3 wires] (1PT23), PT100 in 3 wires";
    case aiot_e::pt1000_3_wires_3:
      return "[3 PT1000 in 3 wires] (3PT33), 3 PT1000 in 3 wires";
    case aiot_e::pt100_3_wires_3:
      return "[3 PT100 in 3 wires] (3PT23), 3 PT100 in 3 wires";
  }
  return "unknown";
}
constexpr auto format_as(aiot_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of alr_e enum decleration
enum struct alr_e : std::uint16_t {
  noa = 0,     ///< No warning stored ([No Warning stored] (NOA))
  frf = 1,     ///< Fallback frequency reaction ([Fallback Frequency] (FRF))
  rls = 2,     ///< Speed maintained ([Speed Maintained] (RLS))
  stt = 3,     ///< Type of stop ([Type of stop] (STT))
  sra = 4,     ///< Reference frequency warning ([Ref Frequency Warning] (SRA))
  lca1 = 5,    ///< Life cycle warning 1 ([Life Cycle Warn 1] (LCA1))
  lca2 = 6,    ///< Life cycle warning 2 ([Life Cycle Warn 2] (LCA2))
  drya = 7,    ///< Dry run warning ([Dry Run Warning] (DRYA))
  lfa = 8,     ///< Low flow warning ([Low Flow Warning] (LFA))
  hfpa = 9,    ///< High flow warning ([High Flow Warning] (HFPA))
  ippa = 10,   ///< Inlet pressure warning ([InPress Warning] (IPPA))
  opla = 11,   ///< Outlet pressure low warning ([Low OutPres Warn] (OPLA))
  opha = 12,   ///< Outlet pressure high warning ([High OutPres Warn] (OPHA))
  pcpa = 13,   ///< Pump cycle warning ([PumpCycle warning] (PCPA))
  jama = 14,   ///< Anti-Jam warning ([Anti-Jam Warning] (JAMA))
  plfa = 15,   ///< Pump low flow  ([Pump Low Flow ] (PLFA))
  lpa = 16,    ///< Low pressure warning ([LowPres Warning] (LPA))
  fsa = 17,    ///< Flow limit activated ([Flow Limit activated] (FSA))
  pee = 18,    ///< PID error warning ([PID error Warning] (PEE))
  pfa = 19,    ///< PID feedback warning ([PID Feedback Warn] (PFA))
  pfah = 20,   ///< PID high feedback warning ([PID High Fdbck Warn] (PFAH))
  pfal = 21,   ///< PID low feedback warning ([PID Low Fdbck Warn] (PFAL))
  pish = 22,   ///< Regulation warning ([Regulation Warning] (PISH))
  tp2a = 26,   ///< AI2 thermal sensor warning ([AI2 Th Warning] (TP2A))
  tp3a = 27,   ///< AI3 thermal sensor warning ([AI3 Th Warning] (TP3A))
  tp4a = 28,   ///< AI4 thermal sensor warning ([AI4 Th Warning] (TP4A))
  tp5a = 29,   ///< AI5 thermal sensor warning ([AI5 Th Warning] (TP5A))
  ap1 = 30,    ///< AI1 4-20 loss warning ([AI1 4-20 Loss Warning] (AP1))
  ap2 = 31,    ///< AI2 4-20 loss warning  ([AI2 4-20 Loss Warning] (AP2))
  ap3 = 32,    ///< AI3 4-20 loss warning ([AI3 4-20 Loss Warning] (AP3))
  ap4 = 33,    ///< AI4 4-20 loss warning ([AI4 4-20 Loss Warning] (AP4))
  ap5 = 34,    ///< AI5 4-20 loss warning ([AI5 4-20 Loss Warning] (AP5))
  tha = 35,    ///< Drive thermal state warning ([Drive Thermal Warning] (THA))
  tja = 36,    ///< IGBT thermal warning ([IGBT Thermal Warning] (TJA))
  fcta = 37,   ///< Fan counter warning ([Fan Counter Warning] (FCTA))
  ffda = 38,   ///< Fan feedback warning ([Fan Feedback Warning] (FFDA))
  efa = 40,    ///< External error warning ([Ext. Error Warning] (EFA))
  usa = 41,    ///< Undervoltage warning ([Undervoltage Warning] (USA))
  upa = 42,    ///< Preventive undervoltage active ([Preventive UnderV Active] (UPA))
  ern = 43,    ///< Drive in forced run ([Forced Run] (ERN))
  fta = 44,    ///< Motor frequency high threshold reached ([Mot Freq High Thd] (FTA))
  ftal = 45,   ///< Motor frequency low threshold reached ([Mot Freq Low Thd] (FTAL))
  f2al = 47,   ///< Motor frequency low threshold 2 reached ([Mot Freq Low Thd 2] (F2AL))
  fla = 48,    ///< High speed reached ([High Speed Reached] (FLA))
  rtah = 49,   ///< Reference frequency high threshold reached ([Ref Freq High Thd reached] (RTAH))
  rtal = 50,   ///< Reference frequency low threshold reached ([Ref Freq Low Thd reached] (RTAL))
  f2a = 51,    ///< 2nd frequency threshold reached ([2nd Freq Thd Reached] (F2A))
  cta = 52,    ///< Current threshold reached ([Current Thd Reached] (CTA))
  ctal = 53,   ///< Low current threshold reached ([Low Current Reached] (CTAL))
  ttha = 54,   ///< High torque warning ([High Torque Warning] (TTHA))
  ttla = 55,   ///< Low torque warning ([Low Torque Warning] (TTLA))
  ula = 56,    ///< Process underload warning ([Process Undld Warning] (ULA))
  ola = 57,    ///< Process overload warning ([Process Overload Warning] (OLA))
  tad = 60,    ///< Drive thermal threshold reached ([Drv Therm Thd reached] (TAD))
  tsa = 61,    ///< Motor thermal threshold reached ([Motor Therm Thd reached] (TSA))
  ptha = 65,   ///< Power high threshold reached ([Power High Threshold] (PTHA))
  pthl = 66,   ///< Power low threshold reached ([Power Low Threshold] (PTHL))
  cas1 = 67,   ///< Customer warning 1 ([Cust Warning 1] (CAS1))
  cas2 = 68,   ///< Customer warning 2 ([Cust Warning 2] (CAS2))
  cas3 = 69,   ///< Customer warning 3 ([Cust Warning 3] (CAS3))
  cas4 = 70,   ///< Customer warning 4 ([Cust Warning 4] (CAS4))
  cas5 = 71,   ///< Customer warning 5 ([Cust Warning 5] (CAS5))
  ura = 72,    ///< AFE Mains undervoltage  ([AFE Mains Undervoltage ] (URA))
  powd = 73,   ///< Power Consumption warning ([Power Cons Warning] (POWD))
  opsa = 74,   ///< Output pressure high switch warning ([Switch OutPres Warn] (OPSA))
  inwm = 75,   ///< Ethernet Internal warning ([Ethernet Internal Warning] (INWM))
  mpca = 78,   ///< Multi-Pump available capacity warning ([MP Capacity Warn] (MPCA))
  mpla = 79,   ///< Lead pump not available ([Lead Pump Warn] (MPLA))
  lcha = 80,   ///< High level warning ([High Level Warning] (LCHA))
  lcla = 81,   ///< Low level warning ([Low Level Warning] (LCLA))
  lcwa = 82,   ///< Level switch warning ([Level Switch Warning] (LCWA))
  iwa = 97,    ///< Monitoring circuit A warning ([MonitorCircuit A Warn] (IWA))
  iwb = 98,    ///< Monitoring circuit B warning ([MonitorCircuit B Warn] (IWB))
  iwc = 99,    ///< Monitoring circuit C warning ([MonitorCircuit C Warn] (IWC))
  iwd = 100,   ///< Monitoring circuit D warning ([MonitorCircuit D Warn] (IWD))
  cwa = 101,   ///< Cabinet circuit A warning ([CabinetCircuit A Warn] (CWA))
  cwb = 102,   ///< Cabinet circuit B warning ([CabinetCircuit B Warn] (CWB))
  cwc = 103,   ///< Cabinet circuit C warning ([CabinetCircuit C Warn] (CWC))
  twa = 104,   ///< Motor winding A warning ([MotorWinding A Warn] (TWA))
  twb = 105,   ///< Motor winding B warning ([MotorWinding B Warn] (TWB))
  twc = 106,   ///< Motor bearing A warning ([MotorBearing A Warn] (TWC))
  twd = 107,   ///< Motor bearing B warning ([MotorBearing B Warn] (TWD))
  cbw = 108,   ///< Circuit breaker warning ([Circuit Breaker Warn] (CBW))
  p24c = 109,  ///< Cabinet I/O 24V missing warning ([Cab I/O 24V Warn] (P24C))
  affl = 110,  ///<  ([None] (AFFL))
  clim = 113,  ///< AFE motor limitation ([AFE Motor Limitation] (CLIM))
  clig = 114,  ///< AFE generator limitation ([AFE Generator Limitation] (CLIG))
  thsa = 115,  ///< AFE sensor thermal state ([AFE Sensor thermal state] (THSA))
  thja = 116,  ///< AFE IGBT thermal state ([AFE IGBT thermal state] (THJA))
  ffca = 117,  ///< Cabinet fan feedback warning ([Cabinet Fan Fdbck Warn] (FFCA))
  fcca = 118,  ///< Cabinet fan counter warning ([Cabinet Fan Counter Warn] (FCCA))
  cha = 119,   ///< Cabinet overheat  warning ([Cabinet Overheat  Warn] (CHA))
  cmij = 120,  ///< CMI jumper warning ([CMI Jumper Warn] (CMIJ))
  fcba = 121,  ///< AFE fan counter warning ([AFE Fan Counter Warn] (FCBA))
  ffba = 122,  ///< AFE fan feedback warning ([AFE Fan Fdbck Warn] (FFBA))
  mpda = 124,  ///< Multipump device warning ([M/P Device Warn] (MPDA))
  ts2a = 134,  ///< Temperature sensor AI2 warning ([Temp Sens AI2 Warn] (TS2A))
  ts3a = 135,  ///< Temperature sensor AI3 warning ([Temp Sens AI3 Warn] (TS3A))
  ts4a = 136,  ///< Temperature sensor AI4 warning ([Temp Sens AI4 Warn] (TS4A))
  ts5a = 137,  ///< Temperature sensor AI5 warning ([Temp Sens AI5 Warn] (TS5A))
  dcrw = 138,  ///< DC bus ripple warning ([DC Bus Ripple Warn] (DCRW))
  copa = 140,  ///< Cooling pump warning ([Cooling Pump Warn] (COPA))
  moa = 142,   ///< Module overheat warning ([Module Overheat] (MOA))
};
[[nodiscard]] constexpr auto enum_desc(alr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case alr_e::noa:
      return "[No Warning stored] (NOA), No warning stored";
    case alr_e::frf:
      return "[Fallback Frequency] (FRF), Fallback frequency reaction";
    case alr_e::rls:
      return "[Speed Maintained] (RLS), Speed maintained";
    case alr_e::stt:
      return "[Type of stop] (STT), Type of stop";
    case alr_e::sra:
      return "[Ref Frequency Warning] (SRA), Reference frequency warning";
    case alr_e::lca1:
      return "[Life Cycle Warn 1] (LCA1), Life cycle warning 1";
    case alr_e::lca2:
      return "[Life Cycle Warn 2] (LCA2), Life cycle warning 2";
    case alr_e::drya:
      return "[Dry Run Warning] (DRYA), Dry run warning";
    case alr_e::lfa:
      return "[Low Flow Warning] (LFA), Low flow warning";
    case alr_e::hfpa:
      return "[High Flow Warning] (HFPA), High flow warning";
    case alr_e::ippa:
      return "[InPress Warning] (IPPA), Inlet pressure warning";
    case alr_e::opla:
      return "[Low OutPres Warn] (OPLA), Outlet pressure low warning";
    case alr_e::opha:
      return "[High OutPres Warn] (OPHA), Outlet pressure high warning";
    case alr_e::pcpa:
      return "[PumpCycle warning] (PCPA), Pump cycle warning";
    case alr_e::jama:
      return "[Anti-Jam Warning] (JAMA), Anti-Jam warning";
    case alr_e::plfa:
      return "[Pump Low Flow ] (PLFA), Pump low flow ";
    case alr_e::lpa:
      return "[LowPres Warning] (LPA), Low pressure warning";
    case alr_e::fsa:
      return "[Flow Limit activated] (FSA), Flow limit activated";
    case alr_e::pee:
      return "[PID error Warning] (PEE), PID error warning";
    case alr_e::pfa:
      return "[PID Feedback Warn] (PFA), PID feedback warning";
    case alr_e::pfah:
      return "[PID High Fdbck Warn] (PFAH), PID high feedback warning";
    case alr_e::pfal:
      return "[PID Low Fdbck Warn] (PFAL), PID low feedback warning";
    case alr_e::pish:
      return "[Regulation Warning] (PISH), Regulation warning";
    case alr_e::tp2a:
      return "[AI2 Th Warning] (TP2A), AI2 thermal sensor warning";
    case alr_e::tp3a:
      return "[AI3 Th Warning] (TP3A), AI3 thermal sensor warning";
    case alr_e::tp4a:
      return "[AI4 Th Warning] (TP4A), AI4 thermal sensor warning";
    case alr_e::tp5a:
      return "[AI5 Th Warning] (TP5A), AI5 thermal sensor warning";
    case alr_e::ap1:
      return "[AI1 4-20 Loss Warning] (AP1), AI1 4-20 loss warning";
    case alr_e::ap2:
      return "[AI2 4-20 Loss Warning] (AP2), AI2 4-20 loss warning ";
    case alr_e::ap3:
      return "[AI3 4-20 Loss Warning] (AP3), AI3 4-20 loss warning";
    case alr_e::ap4:
      return "[AI4 4-20 Loss Warning] (AP4), AI4 4-20 loss warning";
    case alr_e::ap5:
      return "[AI5 4-20 Loss Warning] (AP5), AI5 4-20 loss warning";
    case alr_e::tha:
      return "[Drive Thermal Warning] (THA), Drive thermal state warning";
    case alr_e::tja:
      return "[IGBT Thermal Warning] (TJA), IGBT thermal warning";
    case alr_e::fcta:
      return "[Fan Counter Warning] (FCTA), Fan counter warning";
    case alr_e::ffda:
      return "[Fan Feedback Warning] (FFDA), Fan feedback warning";
    case alr_e::efa:
      return "[Ext. Error Warning] (EFA), External error warning";
    case alr_e::usa:
      return "[Undervoltage Warning] (USA), Undervoltage warning";
    case alr_e::upa:
      return "[Preventive UnderV Active] (UPA), Preventive undervoltage active";
    case alr_e::ern:
      return "[Forced Run] (ERN), Drive in forced run";
    case alr_e::fta:
      return "[Mot Freq High Thd] (FTA), Motor frequency high threshold reached";
    case alr_e::ftal:
      return "[Mot Freq Low Thd] (FTAL), Motor frequency low threshold reached";
    case alr_e::f2al:
      return "[Mot Freq Low Thd 2] (F2AL), Motor frequency low threshold 2 reached";
    case alr_e::fla:
      return "[High Speed Reached] (FLA), High speed reached";
    case alr_e::rtah:
      return "[Ref Freq High Thd reached] (RTAH), Reference frequency high threshold reached";
    case alr_e::rtal:
      return "[Ref Freq Low Thd reached] (RTAL), Reference frequency low threshold reached";
    case alr_e::f2a:
      return "[2nd Freq Thd Reached] (F2A), 2nd frequency threshold reached";
    case alr_e::cta:
      return "[Current Thd Reached] (CTA), Current threshold reached";
    case alr_e::ctal:
      return "[Low Current Reached] (CTAL), Low current threshold reached";
    case alr_e::ttha:
      return "[High Torque Warning] (TTHA), High torque warning";
    case alr_e::ttla:
      return "[Low Torque Warning] (TTLA), Low torque warning";
    case alr_e::ula:
      return "[Process Undld Warning] (ULA), Process underload warning";
    case alr_e::ola:
      return "[Process Overload Warning] (OLA), Process overload warning";
    case alr_e::tad:
      return "[Drv Therm Thd reached] (TAD), Drive thermal threshold reached";
    case alr_e::tsa:
      return "[Motor Therm Thd reached] (TSA), Motor thermal threshold reached";
    case alr_e::ptha:
      return "[Power High Threshold] (PTHA), Power high threshold reached";
    case alr_e::pthl:
      return "[Power Low Threshold] (PTHL), Power low threshold reached";
    case alr_e::cas1:
      return "[Cust Warning 1] (CAS1), Customer warning 1";
    case alr_e::cas2:
      return "[Cust Warning 2] (CAS2), Customer warning 2";
    case alr_e::cas3:
      return "[Cust Warning 3] (CAS3), Customer warning 3";
    case alr_e::cas4:
      return "[Cust Warning 4] (CAS4), Customer warning 4";
    case alr_e::cas5:
      return "[Cust Warning 5] (CAS5), Customer warning 5";
    case alr_e::ura:
      return "[AFE Mains Undervoltage ] (URA), AFE Mains undervoltage ";
    case alr_e::powd:
      return "[Power Cons Warning] (POWD), Power Consumption warning";
    case alr_e::opsa:
      return "[Switch OutPres Warn] (OPSA), Output pressure high switch warning";
    case alr_e::inwm:
      return "[Ethernet Internal Warning] (INWM), Ethernet Internal warning";
    case alr_e::mpca:
      return "[MP Capacity Warn] (MPCA), Multi-Pump available capacity warning";
    case alr_e::mpla:
      return "[Lead Pump Warn] (MPLA), Lead pump not available";
    case alr_e::lcha:
      return "[High Level Warning] (LCHA), High level warning";
    case alr_e::lcla:
      return "[Low Level Warning] (LCLA), Low level warning";
    case alr_e::lcwa:
      return "[Level Switch Warning] (LCWA), Level switch warning";
    case alr_e::iwa:
      return "[MonitorCircuit A Warn] (IWA), Monitoring circuit A warning";
    case alr_e::iwb:
      return "[MonitorCircuit B Warn] (IWB), Monitoring circuit B warning";
    case alr_e::iwc:
      return "[MonitorCircuit C Warn] (IWC), Monitoring circuit C warning";
    case alr_e::iwd:
      return "[MonitorCircuit D Warn] (IWD), Monitoring circuit D warning";
    case alr_e::cwa:
      return "[CabinetCircuit A Warn] (CWA), Cabinet circuit A warning";
    case alr_e::cwb:
      return "[CabinetCircuit B Warn] (CWB), Cabinet circuit B warning";
    case alr_e::cwc:
      return "[CabinetCircuit C Warn] (CWC), Cabinet circuit C warning";
    case alr_e::twa:
      return "[MotorWinding A Warn] (TWA), Motor winding A warning";
    case alr_e::twb:
      return "[MotorWinding B Warn] (TWB), Motor winding B warning";
    case alr_e::twc:
      return "[MotorBearing A Warn] (TWC), Motor bearing A warning";
    case alr_e::twd:
      return "[MotorBearing B Warn] (TWD), Motor bearing B warning";
    case alr_e::cbw:
      return "[Circuit Breaker Warn] (CBW), Circuit breaker warning";
    case alr_e::p24c:
      return "[Cab I/O 24V Warn] (P24C), Cabinet I/O 24V missing warning";
    case alr_e::affl:
      return "[None] (AFFL), ";
    case alr_e::clim:
      return "[AFE Motor Limitation] (CLIM), AFE motor limitation";
    case alr_e::clig:
      return "[AFE Generator Limitation] (CLIG), AFE generator limitation";
    case alr_e::thsa:
      return "[AFE Sensor thermal state] (THSA), AFE sensor thermal state";
    case alr_e::thja:
      return "[AFE IGBT thermal state] (THJA), AFE IGBT thermal state";
    case alr_e::ffca:
      return "[Cabinet Fan Fdbck Warn] (FFCA), Cabinet fan feedback warning";
    case alr_e::fcca:
      return "[Cabinet Fan Counter Warn] (FCCA), Cabinet fan counter warning";
    case alr_e::cha:
      return "[Cabinet Overheat  Warn] (CHA), Cabinet overheat  warning";
    case alr_e::cmij:
      return "[CMI Jumper Warn] (CMIJ), CMI jumper warning";
    case alr_e::fcba:
      return "[AFE Fan Counter Warn] (FCBA), AFE fan counter warning";
    case alr_e::ffba:
      return "[AFE Fan Fdbck Warn] (FFBA), AFE fan feedback warning";
    case alr_e::mpda:
      return "[M/P Device Warn] (MPDA), Multipump device warning";
    case alr_e::ts2a:
      return "[Temp Sens AI2 Warn] (TS2A), Temperature sensor AI2 warning";
    case alr_e::ts3a:
      return "[Temp Sens AI3 Warn] (TS3A), Temperature sensor AI3 warning";
    case alr_e::ts4a:
      return "[Temp Sens AI4 Warn] (TS4A), Temperature sensor AI4 warning";
    case alr_e::ts5a:
      return "[Temp Sens AI5 Warn] (TS5A), Temperature sensor AI5 warning";
    case alr_e::dcrw:
      return "[DC Bus Ripple Warn] (DCRW), DC bus ripple warning";
    case alr_e::copa:
      return "[Cooling Pump Warn] (COPA), Cooling pump warning";
    case alr_e::moa:
      return "[Module Overheat] (MOA), Module overheat warning";
  }
  return "unknown";
}
constexpr auto format_as(alr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of apps_e enum decleration
enum struct apps_e : std::uint16_t {
  run = 0,        ///< Running  ([Running ] (RUN))
  stop = 1,       ///< Stop ([Stop] (STOP))
  local = 2,      ///< Local mode is active ([Local Mode Active] (LOCAL))
  over = 3,       ///< Channel 2 active ([Channel 2 Active] (OVER))
  manu = 4,       ///< Manual mode is active ([Manual Mode Active] (MANU))
  automatic = 5,  ///< PID active ([PID Active] (AUTO))
  ajam = 6,       ///< Anti-Jam is in progress ([Anti-Jam In progress] (AJAM))
  flim = 7,       ///< Flow limit In progress ([Flow limit In progress] (FLIM))
  fill = 8,       ///< PipeFill is in progress ([PipeFill In progress] (FILL))
  jockey = 9,     ///< Jockey pump is active ([Jockey Pump Active] (JOCKEY))
  boost = 10,     ///< Boost is in progress ([Boost In progress] (BOOST))
  sleep = 11,     ///< Sleep mode is active ([Sleep Active] (SLEEP))
  prim = 12,      ///< Priming pump is active ([Priming Pump Active] (PRIM))
  comp = 13,      ///< Inlet pressure Compensation is in progress ([InletPres Comp Active] (COMP))
  undef = 15,     ///< Undefined ([Undefined] (UNDEF))
};
[[nodiscard]] constexpr auto enum_desc(apps_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case apps_e::run:
      return "[Running ] (RUN), Running ";
    case apps_e::stop:
      return "[Stop] (STOP), Stop";
    case apps_e::local:
      return "[Local Mode Active] (LOCAL), Local mode is active";
    case apps_e::over:
      return "[Channel 2 Active] (OVER), Channel 2 active";
    case apps_e::manu:
      return "[Manual Mode Active] (MANU), Manual mode is active";
    case apps_e::automatic:
      return "[PID Active] (AUTO), PID active";
    case apps_e::ajam:
      return "[Anti-Jam In progress] (AJAM), Anti-Jam is in progress";
    case apps_e::flim:
      return "[Flow limit In progress] (FLIM), Flow limit In progress";
    case apps_e::fill:
      return "[PipeFill In progress] (FILL), PipeFill is in progress";
    case apps_e::jockey:
      return "[Jockey Pump Active] (JOCKEY), Jockey pump is active";
    case apps_e::boost:
      return "[Boost In progress] (BOOST), Boost is in progress";
    case apps_e::sleep:
      return "[Sleep Active] (SLEEP), Sleep mode is active";
    case apps_e::prim:
      return "[Priming Pump Active] (PRIM), Priming pump is active";
    case apps_e::comp:
      return "[InletPres Comp Active] (COMP), Inlet pressure Compensation is in progress";
    case apps_e::undef:
      return "[Undefined] (UNDEF), Undefined";
  }
  return "unknown";
}
constexpr auto format_as(apps_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of appt_e enum decleration
enum struct appt_e : std::uint16_t {
  gpmp = 0,   ///< Generic pump Control ([Generic Pump Control] (GPMP))
  level = 1,  ///< Pump level control application ([Pump Level Control] (LEVEL))
  boost = 2,  ///< Pump booster control application ([Pump Booster Control] (BOOST))
  fan = 3,    ///< Generic fan control application ([Generic Fan Control] (FAN))
};
[[nodiscard]] constexpr auto enum_desc(appt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case appt_e::gpmp:
      return "[Generic Pump Control] (GPMP), Generic pump Control";
    case appt_e::level:
      return "[Pump Level Control] (LEVEL), Pump level control application";
    case appt_e::boost:
      return "[Pump Booster Control] (BOOST), Pump booster control application";
    case appt_e::fan:
      return "[Generic Fan Control] (FAN), Generic fan control application";
  }
  return "unknown";
}
constexpr auto format_as(appt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ast_e enum decleration
enum struct ast_e : std::uint16_t {
  psi = 5,   ///< Pulse Signal injection ([PSI align.] (PSI))
  psio = 6,  ///< Pulse Signal injection - Optimized ([PSIO align.] (PSIO))
  rci = 7,   ///< Rotational current injection ([Rotational Current Injection] (RCI))
  no = 254,  ///< NO alignment ([No align.] (NO))
};
[[nodiscard]] constexpr auto enum_desc(ast_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ast_e::psi:
      return "[PSI align.] (PSI), Pulse Signal injection";
    case ast_e::psio:
      return "[PSIO align.] (PSIO), Pulse Signal injection - Optimized";
    case ast_e::rci:
      return "[Rotational Current Injection] (RCI), Rotational current injection";
    case ast_e::no:
      return "[No align.] (NO), NO alignment";
  }
  return "unknown";
}
constexpr auto format_as(ast_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of aut_e enum decleration
enum struct aut_e : std::uint16_t {
  no = 0,   ///< No ([No] (NO))
  yes = 1,  ///< Yes ([Yes] (YES))
};
[[nodiscard]] constexpr auto enum_desc(aut_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case aut_e::no:
      return "[No] (NO), No";
    case aut_e::yes:
      return "[Yes] (YES), Yes";
  }
  return "unknown";
}
constexpr auto format_as(aut_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of avot_e enum decleration
enum struct avot_e : std::uint16_t {
  ineg = 0,  ///< +/- 8192 ([bipolar 8192] (INEG))
  pneg = 1,  ///< +/- 100% ([bipolar 100] (PNEG))
};
[[nodiscard]] constexpr auto enum_desc(avot_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case avot_e::ineg:
      return "[bipolar 8192] (INEG), +/- 8192";
    case avot_e::pneg:
      return "[bipolar 100] (PNEG), +/- 100%";
  }
  return "unknown";
}
constexpr auto format_as(avot_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bcs_e enum decleration
enum struct bcs_e : std::uint16_t {
  none = 0,   ///< Not configured ([None] (NONE))
  nact = 1,   ///< Inactive ([Inactive] (NACT))
  run = 2,    ///< Running ([Running] (RUN))
  stgp = 3,   ///< Stage Pending ([Stage Pending] (STGP))
  dstgp = 4,  ///< Destage pending ([Destage pending] (DSTGP))
  stg = 5,    ///< Staging ([Staging] (STG))
  dstg = 6,   ///< Destage in progress ([Destaging] (DSTG))
};
[[nodiscard]] constexpr auto enum_desc(bcs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bcs_e::none:
      return "[None] (NONE), Not configured";
    case bcs_e::nact:
      return "[Inactive] (NACT), Inactive";
    case bcs_e::run:
      return "[Running] (RUN), Running";
    case bcs_e::stgp:
      return "[Stage Pending] (STGP), Stage Pending";
    case bcs_e::dstgp:
      return "[Destage pending] (DSTGP), Destage pending";
    case bcs_e::stg:
      return "[Staging] (STG), Staging";
    case bcs_e::dstg:
      return "[Destaging] (DSTG), Destage in progress";
  }
  return "unknown";
}
constexpr auto format_as(bcs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bdco_e enum decleration
enum struct bdco_e : std::uint16_t {
  kbps_50 = 38,   ///< Baud rate 50kbps ([50 kbps] (50K))
  kbps_125 = 52,  ///< Baud rate 125kbps ([125 kbps] (125K))
  kbps_250 = 60,  ///< Baud rate 250kbps ([250 kbps] (250K))
  kbps_500 = 68,  ///< Baud rate 500kbps ([500 kbps] (500K))
  mbps_1 = 76,    ///< Baud rate 1Mbps ([1 Mbps] (1M))
};
[[nodiscard]] constexpr auto enum_desc(bdco_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bdco_e::kbps_50:
      return "[50 kbps] (50 ), ";
    case bdco_e::kbps_125:
      return "[125 kbps] (125 ), ";
    case bdco_e::kbps_250:
      return "[250 kbps] (250 ), ";
    case bdco_e::kbps_500:
      return "[500 kbps] (500 ), ";
    case bdco_e::mbps_1:
      return "[1 Mbps] (1M), ";
  }
  return "unknown";
}
constexpr auto format_as(bdco_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bfr_e enum decleration
enum struct bfr_e : std::uint16_t {
  iec = 0,   ///< 50Hz motor frequency ([50Hz IEC] (50Hz))
  nema = 1,  ///< 60Hz motor frequency ([60Hz NEMA] (60Hz))
};
[[nodiscard]] constexpr auto enum_desc(bfr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bfr_e::iec:
      return "[50Hz IEC] (50Hz), 50Hz motor frequency";
    case bfr_e::nema:
      return "[60Hz NEMA] (60Hz), 60Hz motor frequency";
  }
  return "unknown";
}
constexpr auto format_as(bfr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bmp_e enum decleration
enum struct bmp_e : std::uint16_t {
  stop = 0,  ///< Cmd/ref clear on c/over ([Stop] (STOP))
  bump = 1,  ///< Cmd/ref copied on c/over ([Bumpless] (BUMP))
  dis = 2,   ///< Disabled ([Disabled] (DIS))
};
[[nodiscard]] constexpr auto enum_desc(bmp_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bmp_e::stop:
      return "[Stop] (STOP), Cmd/ref clear on c/over";
    case bmp_e::bump:
      return "[Bumpless] (BUMP), Cmd/ref copied on c/over";
    case bmp_e::dis:
      return "[Disabled] (DIS), Disabled";
  }
  return "unknown";
}
constexpr auto format_as(bmp_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of boa_e enum decleration
enum struct boa_e : std::uint16_t {
  no = 0,    ///< Inactive ([Inactive] (NO))
  dyna = 1,  ///< Dynamic ([Dynamic] (DYNA))
  stat = 2,  ///< Static ([Static] (STAT))
  cste = 3,  ///< Constant ([Constant] (CSTE))
};
[[nodiscard]] constexpr auto enum_desc(boa_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case boa_e::no:
      return "[Inactive] (NO), Inactive";
    case boa_e::dyna:
      return "[Dynamic] (DYNA), Dynamic";
    case boa_e::stat:
      return "[Static] (STAT), Static";
    case boa_e::cste:
      return "[Constant] (CSTE), Constant";
  }
  return "unknown";
}
constexpr auto format_as(boa_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bra_e enum decleration
enum struct bra_e : std::uint16_t {
  no = 0,    ///< No ([No] (NO))
  yes = 1,   ///< Yes ([Yes] (YES))
  dyna = 2,  ///< High torque ([High Torque] (DYNA))
};
[[nodiscard]] constexpr auto enum_desc(bra_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bra_e::no:
      return "[No] (NO), No";
    case bra_e::yes:
      return "[Yes] (YES), Yes";
    case bra_e::dyna:
      return "[High Torque] (DYNA), High torque";
  }
  return "unknown";
}
constexpr auto format_as(bra_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bsdc_e enum decleration
enum struct bsdc_e : std::uint16_t {
  spd = 0,   ///< Speed ([Speed] (SPD))
  fbk = 1,   ///< Feedback ([Feedback] (FBK))
  spfl = 2,  ///< Speed+Flow ([Speed+Flow] (SPFL))
  fbfl = 3,  ///< Feedback+Flow ([Feedback+Flow] (FBFL))
  opt = 5,   ///< Energy Optimized  ([Energy Optimized ] (OPT))
};
[[nodiscard]] constexpr auto enum_desc(bsdc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bsdc_e::spd:
      return "[Speed] (SPD), Speed";
    case bsdc_e::fbk:
      return "[Feedback] (FBK), Feedback";
    case bsdc_e::spfl:
      return "[Speed+Flow] (SPFL), Speed+Flow";
    case bsdc_e::fbfl:
      return "[Feedback+Flow] (FBFL), Feedback+Flow";
    case bsdc_e::opt:
      return "[Energy Optimized ] (OPT), Energy Optimized ";
  }
  return "unknown";
}
constexpr auto format_as(bsdc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bsdm_e enum decleration
enum struct bsdm_e : std::uint16_t {
  bspd = 0,  ///<  ([Speed] (BSPD))
  bfbk = 1,  ///<  ([Feedback] (BFBK))
  advc = 2,  ///<  ([Advanced] (ADVC))
};
[[nodiscard]] constexpr auto enum_desc(bsdm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bsdm_e::bspd:
      return "[Speed] (BSPD), ";
    case bsdm_e::bfbk:
      return "[Feedback] (BFBK), ";
    case bsdm_e::advc:
      return "[Advanced] (ADVC), ";
  }
  return "unknown";
}
constexpr auto format_as(bsdm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of bsp_e enum decleration
enum struct bsp_e : std::uint16_t {
  bsd = 0,   ///< Standard ref template ([Standard] (BSD))
  bls = 1,   ///< Pedestal at LSP ([Pedestal] (BLS))
  bns = 2,   ///< Deadband at LSP ([Deadband] (BNS))
  bns0 = 4,  ///< Deadband at 0 speed ([Deadband at 0%] (BNS0))
};
[[nodiscard]] constexpr auto enum_desc(bsp_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case bsp_e::bsd:
      return "[Standard] (BSD), Standard ref template";
    case bsp_e::bls:
      return "[Pedestal] (BLS), Pedestal at LSP";
    case bsp_e::bns:
      return "[Deadband] (BNS), Deadband at LSP";
    case bsp_e::bns0:
      return "[Deadband at 0%] (BNS0), Deadband at 0 speed";
  }
  return "unknown";
}
constexpr auto format_as(bsp_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of car_e enum decleration
enum struct car_e : std::uint16_t {
  no = 0,   ///< No warning clearing ([No Warning Clearing] (NO))
  ra1 = 1,  ///< Clear Event 1 warning ([Clear Event 1 Warning] (RA1))
  ra2 = 2,  ///< Clear Event 2 warning ([Clear Event 2 Warning] (RA2))
  ra3 = 3,  ///< Clear Event 3 warning ([Clear Event 3 Warning] (RA3))
  ra4 = 4,  ///< Clear Event 4 warning ([Clear Event 4 Warning] (RA4))
  ra5 = 5,  ///< Clear Event 5 warning ([Clear Event 5 Warning] (RA5))
};
[[nodiscard]] constexpr auto enum_desc(car_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case car_e::no:
      return "[No Warning Clearing] (NO), No warning clearing";
    case car_e::ra1:
      return "[Clear Event 1 Warning] (RA1), Clear Event 1 warning";
    case car_e::ra2:
      return "[Clear Event 2 Warning] (RA2), Clear Event 2 warning";
    case car_e::ra3:
      return "[Clear Event 3 Warning] (RA3), Clear Event 3 warning";
    case car_e::ra4:
      return "[Clear Event 4 Warning] (RA4), Clear Event 4 warning";
    case car_e::ra5:
      return "[Clear Event 5 Warning] (RA5), Clear Event 5 warning";
  }
  return "unknown";
}
constexpr auto format_as(car_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cbs_e enum decleration
enum struct cbs_e : std::uint16_t {
  no = 0,    ///< Circuit breaker not configured ([CB Not Configured] (NO))
  cbci = 1,  ///< Circuit breaker configuration invalid ([CB Invalid Config ] (CBCI))
  cbst = 2,  ///< Circuit breaker in start pulse ([CB In Start Pulse] (CBST))
  cbnc = 3,  ///< Circuit breaker not closed ([CB Not Closed] (CBNC))
  cbos = 4,  ///< Circuit breaker open ([CB Open] (CBOS))
  cbsp = 5,  ///< Circuit breaker in stop pulse ([CB In Stop Pulse] (CBSP))
  cbno = 6,  ///< Circuit breaker not open ([CB Not Open] (CBNO))
  cbcs = 7,  ///< Circuit breaker closed ([CB Closed] (CBCS))
  cbsd = 8,  ///< Circuit breaker stop is disable ([CB stop disable] (CBSD))
};
[[nodiscard]] constexpr auto enum_desc(cbs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cbs_e::no:
      return "[CB Not Configured] (NO), Circuit breaker not configured";
    case cbs_e::cbci:
      return "[CB Invalid Config ] (CBCI), Circuit breaker configuration invalid";
    case cbs_e::cbst:
      return "[CB In Start Pulse] (CBST), Circuit breaker in start pulse";
    case cbs_e::cbnc:
      return "[CB Not Closed] (CBNC), Circuit breaker not closed";
    case cbs_e::cbos:
      return "[CB Open] (CBOS), Circuit breaker open";
    case cbs_e::cbsp:
      return "[CB In Stop Pulse] (CBSP), Circuit breaker in stop pulse";
    case cbs_e::cbno:
      return "[CB Not Open] (CBNO), Circuit breaker not open";
    case cbs_e::cbcs:
      return "[CB Closed] (CBCS), Circuit breaker closed";
    case cbs_e::cbsd:
      return "[CB stop disable] (CBSD), Circuit breaker stop is disable";
  }
  return "unknown";
}
constexpr auto format_as(cbs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cbsr_e enum decleration
enum struct cbsr_e : std::uint16_t {
  flt = 0,  ///< Error ([Error] (FLT))
  war = 1,  ///< warning ([Warning] (WAR))
};
[[nodiscard]] constexpr auto enum_desc(cbsr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cbsr_e::flt:
      return "[Error] (FLT), Error";
    case cbsr_e::war:
      return "[Warning] (WAR), warning";
  }
  return "unknown";
}
constexpr auto format_as(cbsr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cca_e enum decleration
enum struct cca_e : std::uint16_t {
  no = 0,   ///< Not Configured ([Not Configured] (NO))
  cpt = 1,  ///< Counter ([Counter] (CPT))
  dt = 2,   ///< Date and time ([Date and Time] (DT))
};
[[nodiscard]] constexpr auto enum_desc(cca_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cca_e::no:
      return "[Not Configured] (NO), Not Configured";
    case cca_e::cpt:
      return "[Counter] (CPT), Counter";
    case cca_e::dt:
      return "[Date and Time] (DT), Date and time";
  }
  return "unknown";
}
constexpr auto format_as(cca_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ccs_e enum decleration
enum struct ccs_e : std::uint16_t {
  mains_or_control_supply_on = 0,  ///< Mains or Control Supply ON ([Mains/Control ON] (0))
  mains_supply_on = 1,             ///< Mains Supply ON ([Mains Supply ON] (1))
  drive_in_running_state = 2,      ///< Drive in running State ([Drive is Running] (2))
};
[[nodiscard]] constexpr auto enum_desc(ccs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ccs_e::mains_or_control_supply_on:
      return "[Mains/Control ON] (0), Mains or Control Supply ON";
    case ccs_e::mains_supply_on:
      return "[Mains Supply ON] (1), Mains Supply ON";
    case ccs_e::drive_in_running_state:
      return "[Drive is Running] (2), Drive in running State";
  }
  return "unknown";
}
constexpr auto format_as(ccs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cdx_e enum decleration
enum struct cdx_e : std::uint16_t {
  ter = 1,   ///< Terminal block ([Terminals] (TER))
  lcc = 3,   ///< Local HMI ([HMI] (LCC))
  mdb = 10,  ///< Modbus communication ([Modbus] (MDB))
  can = 20,  ///< CANopen communication ([CANopen] (CAN))
  net = 30,  ///< Ext. communication module ([Com. Module] (NET))
  eth = 40,  ///< Ethernet ([Ethernet] (ETH))
};
[[nodiscard]] constexpr auto enum_desc(cdx_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cdx_e::ter:
      return "[Terminals] (TER), Terminal block";
    case cdx_e::lcc:
      return "[HMI] (LCC), Local HMI";
    case cdx_e::mdb:
      return "[Modbus] (MDB), Modbus communication";
    case cdx_e::can:
      return "[CANopen] (CAN), CANopen communication";
    case cdx_e::net:
      return "[Com. Module] (NET), Ext. communication module";
    case cdx_e::eth:
      return "[Ethernet] (ETH), Ethernet";
  }
  return "unknown";
}
constexpr auto format_as(cdx_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cfps_e enum decleration
enum struct cfps_e : std::uint16_t {
  no = 0,    ///< Not Assigned ([None] (NO))
  cfp1 = 1,  ///< Parameter set 1 ([Set No.1] (CFP1))
  cfp2 = 2,  ///< Parameter set 2 ([Set No.2] (CFP2))
  cfp3 = 3,  ///< Parameter set 3 ([Set No.3] (CFP3))
};
[[nodiscard]] constexpr auto enum_desc(cfps_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cfps_e::no:
      return "[None] (NO), Not Assigned";
    case cfps_e::cfp1:
      return "[Set No.1] (CFP1), Parameter set 1";
    case cfps_e::cfp2:
      return "[Set No.2] (CFP2), Parameter set 2";
    case cfps_e::cfp3:
      return "[Set No.3] (CFP3), Parameter set 3";
  }
  return "unknown";
}
constexpr auto format_as(cfps_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of chcf_e enum decleration
enum struct chcf_e : std::uint16_t {
  sim = 1,  ///< Combined channel mode ([Not separ.] (SIM))
  sep = 2,  ///< Separated channel mode ([Separate] (SEP))
  io = 3,   ///< I/O mode ([I/O profile] (IO))
};
[[nodiscard]] constexpr auto enum_desc(chcf_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case chcf_e::sim:
      return "[Not separ.] (SIM), Combined channel mode";
    case chcf_e::sep:
      return "[Separate] (SEP), Separated channel mode";
    case chcf_e::io:
      return "[I/O profile] (IO), I/O mode";
  }
  return "unknown";
}
constexpr auto format_as(chcf_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of chr_e enum decleration
enum struct chr_e : std::uint16_t {
  no = 0,     ///< No ([No] (NO))
  alrm = 1,   ///< warning ([Warning] (ALRM))
  flt = 2,    ///< Error ([Error] (FLT))
  alflt = 3,  ///<  ([None] (ALFLT))
};
[[nodiscard]] constexpr auto enum_desc(chr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case chr_e::no:
      return "[No] (NO), No";
    case chr_e::alrm:
      return "[Warning] (ALRM), warning";
    case chr_e::flt:
      return "[Error] (FLT), Error";
    case chr_e::alflt:
      return "[None] (ALFLT), ";
  }
  return "unknown";
}
constexpr auto format_as(chr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cinr_e enum decleration
enum struct cinr_e : std::uint16_t {
  milli = 20,  ///< 0.001 ([0.001] (0001))
  centi = 30,  ///< 0.01 ([0.01] (001))
  deci = 40,   ///< 0.1 ([0.1] (01))
  base = 50,   ///< 1 ([1] (1))
  deka = 60,   ///< 10 ([10] (10))
};
[[nodiscard]] constexpr auto enum_desc(cinr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cinr_e::milli:
      return "[0.001] (0001), 0.001";
    case cinr_e::centi:
      return "[0.01] (001), 0.01";
    case cinr_e::deci:
      return "[0.1] (01), 0.1";
    case cinr_e::base:
      return "[1] (1), 1";
    case cinr_e::deka:
      return "[10] (10), 10";
  }
  return "unknown";
}
constexpr auto format_as(cinr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cioa_e enum decleration
enum struct cioa_e : std::uint16_t {
  over_20_70 = 0,    ///< 20/70 ([20/70] (20))
  over_21_71 = 1,    ///< 21/71 ([21/71] (21))
  over_100_101 = 2,  ///< 100/101 ([100/101] (100))
  uncg = 3,          ///< Unconfigured ([Unconfig.] (UNCG))
};
[[nodiscard]] constexpr auto enum_desc(cioa_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cioa_e::over_20_70:
      return "[20/70] (20), 20/70";
    case cioa_e::over_21_71:
      return "[21/71] (21), 21/71";
    case cioa_e::over_100_101:
      return "[100/101] (100), 100/101";
    case cioa_e::uncg:
      return "[Unconfig.] (UNCG), Unconfigured";
  }
  return "unknown";
}
constexpr auto format_as(cioa_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cmdt_e enum decleration
enum struct cmdt_e : std::uint16_t {
  no = 0,   ///< Parameters list ([Parameters List] (NO))
  pft = 1,  ///< PID feedback trend view ([PID Feedback] (PFT))
  opt = 2,  ///< Outlet pressure trend view ([Outlet Pressure] (OPT))
  ipt = 3,  ///< Inlet pressure trend view ([Inlet Pressure] (IPT))
  ift = 4,  ///< Installation flow trend view ([Installation Flow] (IFT))
};
[[nodiscard]] constexpr auto enum_desc(cmdt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cmdt_e::no:
      return "[Parameters List] (NO), Parameters list";
    case cmdt_e::pft:
      return "[PID Feedback] (PFT), PID feedback trend view";
    case cmdt_e::opt:
      return "[Outlet Pressure] (OPT), Outlet pressure trend view";
    case cmdt_e::ipt:
      return "[Inlet Pressure] (IPT), Inlet pressure trend view";
    case cmdt_e::ift:
      return "[Installation Flow] (IFT), Installation flow trend view";
  }
  return "unknown";
}
constexpr auto format_as(cmdt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cnfs_e enum decleration
enum struct cnfs_e : std::uint16_t {
  no = 0,    ///< In progress ([In progress] (NO))
  cnf0 = 1,  ///< Configuration 0 active ([Config. No.0] (CNF0))
};
[[nodiscard]] constexpr auto enum_desc(cnfs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cnfs_e::no:
      return "[In progress] (NO), In progress";
    case cnfs_e::cnf0:
      return "[Config. No.0] (CNF0), Configuration 0 active";
  }
  return "unknown";
}
constexpr auto format_as(cnfs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cnl_e enum decleration
enum struct cnl_e : std::uint16_t {
  ter = 0,   ///< Terminal block ([Terminals] (TER))
  lcc = 2,   ///< Local HMI ([HMI] (LCC))
  mdb = 3,   ///< Modbus communication 1 ([Modbus] (MDB))
  can = 6,   ///< CANopen communication ([CANopen] (CAN))
  net = 9,   ///< Ext. communication module ([Com. Module] (NET))
  eth = 11,  ///< Ethernet option module ([Ethernet Module] (ETH))
  pws = 15,  ///< PC tool ([PC tool] (PWS))
};
[[nodiscard]] constexpr auto enum_desc(cnl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cnl_e::ter:
      return "[Terminals] (TER), Terminal block";
    case cnl_e::lcc:
      return "[HMI] (LCC), Local HMI";
    case cnl_e::mdb:
      return "[Modbus] (MDB), Modbus communication 1";
    case cnl_e::can:
      return "[CANopen] (CAN), CANopen communication";
    case cnl_e::net:
      return "[Com. Module] (NET), Ext. communication module";
    case cnl_e::eth:
      return "[Ethernet Module] (ETH), Ethernet option module";
    case cnl_e::pws:
      return "[PC tool] (PWS), PC tool";
  }
  return "unknown";
}
constexpr auto format_as(cnl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cofm_e enum decleration
enum struct cofm_e : std::uint16_t {
  hwcof = 0,  ///< Measured ([Measured] (HWCOF))
  swcof = 1,  ///< Computed ([Computed] (SWCOF))
};
[[nodiscard]] constexpr auto enum_desc(cofm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cofm_e::hwcof:
      return "[Measured] (HWCOF), Measured";
    case cofm_e::swcof:
      return "[Computed] (SWCOF), Computed";
  }
  return "unknown";
}
constexpr auto format_as(cofm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of com1_e enum decleration
enum struct com1_e : std::uint16_t {
  r0t0 = 0,  ///< R0T0 ([R0T0] (R0T0))
  r0t1 = 1,  ///< R0T1 ([R0T1] (R0T1))
  r1t0 = 2,  ///< R1T0 ([R1T0] (R1T0))
  r1t1 = 3,  ///< R1T1 ([R1T1] (R1T1))
};
[[nodiscard]] constexpr auto enum_desc(com1_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case com1_e::r0t0:
      return "[R0T0] (R0T0), R0T0";
    case com1_e::r0t1:
      return "[R0T1] (R0T1), R0T1";
    case com1_e::r1t0:
      return "[R1T0] (R1T0), R1T0";
    case com1_e::r1t1:
      return "[R1T1] (R1T1), R1T1";
  }
  return "unknown";
}
constexpr auto format_as(com1_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cop_e enum decleration
enum struct cop_e : std::uint16_t {
  no = 0,   ///< No copy ([No] (NO))
  sp = 1,   ///< Copy reference frequency ([Reference Frequency] (SP))
  cd = 2,   ///< Copy command ([Command] (CD))
  all = 3,  ///< Copy command & reference frequency ([Cmd + Ref Frequency] (ALL))
};
[[nodiscard]] constexpr auto enum_desc(cop_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cop_e::no:
      return "[No] (NO), No copy";
    case cop_e::sp:
      return "[Reference Frequency] (SP), Copy reference frequency";
    case cop_e::cd:
      return "[Command] (CD), Copy command";
    case cop_e::all:
      return "[Cmd + Ref Frequency] (ALL), Copy command & reference frequency";
  }
  return "unknown";
}
constexpr auto format_as(cop_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of csa_e enum decleration
enum struct csa_e : std::uint16_t {
  no = 0,        ///< Not assigned ([No] (NO))
  ao1 = 1,       ///< AQ1 assignment ([AQ1 assignment] (AO1))
  ao2 = 2,       ///< AQ2 assignment ([AQ2 assignment] (AO2))
  aifr1 = 129,   ///< Reference frequency 1 ([Ref Frequency 1] (AIFR1))
  aifr2 = 130,   ///< Reference frequency 2 ([Ref Frequency 2] (AIFR2))
  aisa2 = 131,   ///< Reference frequency 2 Summing ([Ref Frequency 2 Summing] (AISA2))
  aipif = 132,   ///< PI controller feedback ([PID feedback] (AIPIF))
  aida2 = 137,   ///< Subtract reference frequency 2 ([Subtract Ref Freq 2] (AIDA2))
  aipim = 138,   ///< Manual PID reference ([Manual PID ref.] (AIPIM))
  aifpi = 139,   ///< PID reference frequency ([PID Ref Frequency] (AIFPI))
  aisa3 = 160,   ///< Reference frequency 3 Summing ([Ref Frequency 3 Summing] (AISA3))
  aifr1b = 161,  ///< Reference frequency 1B ([Ref Frequency 1B] (AIFR1B))
  aida3 = 162,   ///< Subtract reference frequency 3 ([Subtract Ref Freq 3] (AIDA3))
  aifloc = 163,  ///< Forced loc mode channel ([Forced local] (AIFLOC))
  aima2 = 164,   ///< Reference frequency 2 multiplier ([Ref Frequency 2 multiplier] (AIMA2))
  aima3 = 165,   ///< Reference frequency 3 multiplier ([Ref Frequency 3 multiplier] (AIMA3))
  aiaic1 = 168,  ///< Virtual AI1 channel  ([Virtual AI1 Channel ] (AIAIC1))
  aiaic2 = 170,  ///< Virtual AI2 Channel ([Virtual AI2 Channel] (AIAIC2))
  aiaic3 = 171,  ///< Virtual AI3 Channel ([Virtual AI3 Channel] (AIAIC3))
  aiteff = 205,  ///< External feed forward ([External Feed Forward] (AITEFF))
  ps1a = 340,    ///< Intlet pressure sensor source ([InletPres Sensor] (PS1A))
  ps2a = 341,    ///< Outlet pressure sensor source ([OutletPres Sensor] (PS2A))
  fs1a = 342,    ///< Installation flow sensor source ([Inst Flow Sensor] (FS1A))
  fs2a = 343,    ///< Pump flow sensor source ([Pump Flow Sensor] (FS2A))
  lcsa = 344,    ///< Level Control sensor ([LevelCtrl Sensor] (LCSA))
};
[[nodiscard]] constexpr auto enum_desc(csa_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case csa_e::no:
      return "[No] (NO), Not assigned";
    case csa_e::ao1:
      return "[AQ1 assignment] (AO1), AQ1 assignment";
    case csa_e::ao2:
      return "[AQ2 assignment] (AO2), AQ2 assignment";
    case csa_e::aifr1:
      return "[Ref Frequency 1] (AIFR1), Reference frequency 1";
    case csa_e::aifr2:
      return "[Ref Frequency 2] (AIFR2), Reference frequency 2";
    case csa_e::aisa2:
      return "[Ref Frequency 2 Summing] (AISA2), Reference frequency 2 Summing";
    case csa_e::aipif:
      return "[PID feedback] (AIPIF), PI controller feedback";
    case csa_e::aida2:
      return "[Subtract Ref Freq 2] (AIDA2), Subtract reference frequency 2";
    case csa_e::aipim:
      return "[Manual PID ref.] (AIPIM), Manual PID reference";
    case csa_e::aifpi:
      return "[PID Ref Frequency] (AIFPI), PID reference frequency";
    case csa_e::aisa3:
      return "[Ref Frequency 3 Summing] (AISA3), Reference frequency 3 Summing";
    case csa_e::aifr1b:
      return "[Ref Frequency 1B] (AIFR1B), Reference frequency 1B";
    case csa_e::aida3:
      return "[Subtract Ref Freq 3] (AIDA3), Subtract reference frequency 3";
    case csa_e::aifloc:
      return "[Forced local] (AIFLOC), Forced loc mode channel";
    case csa_e::aima2:
      return "[Ref Frequency 2 multiplier] (AIMA2), Reference frequency 2 multiplier";
    case csa_e::aima3:
      return "[Ref Frequency 3 multiplier] (AIMA3), Reference frequency 3 multiplier";
    case csa_e::aiaic1:
      return "[Virtual AI1 Channel ] (AIAIC1), Virtual AI1 channel ";
    case csa_e::aiaic2:
      return "[Virtual AI2 Channel] (AIAIC2), Virtual AI2 Channel";
    case csa_e::aiaic3:
      return "[Virtual AI3 Channel] (AIAIC3), Virtual AI3 Channel";
    case csa_e::aiteff:
      return "[External Feed Forward] (AITEFF), External feed forward";
    case csa_e::ps1a:
      return "[InletPres Sensor] (PS1A), Intlet pressure sensor source";
    case csa_e::ps2a:
      return "[OutletPres Sensor] (PS2A), Outlet pressure sensor source";
    case csa_e::fs1a:
      return "[Inst Flow Sensor] (FS1A), Installation flow sensor source";
    case csa_e::fs2a:
      return "[Pump Flow Sensor] (FS2A), Pump flow sensor source";
    case csa_e::lcsa:
      return "[LevelCtrl Sensor] (LCSA), Level Control sensor";
  }
  return "unknown";
}
constexpr auto format_as(csa_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cslfn_e enum decleration
enum struct cslfn_e : std::uint16_t {
  no = 0,       ///< No ([No] (NO))
  fnps1 = 181,  ///< Preset speed 1 ([Preset Speed 1] (FNPS1))
  fnps2 = 182,  ///< Preset speed 2 ([Preset Speed 2] (FNPS2))
  fnpr1 = 183,  ///< PID reference frequency 1 ([PID Ref Freq 1] (FNPR1))
  fnpr2 = 184,  ///< PID reference frequency 2 ([PID Ref Freq 2] (FNPR2))
  fnusp = 185,  ///< Increase speed ([+speed] (FNUSP))
  fndsp = 186,  ///< Decrease speed ([-speed] (FNDSP))
};
[[nodiscard]] constexpr auto enum_desc(cslfn_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cslfn_e::no:
      return "[No] (NO), No";
    case cslfn_e::fnps1:
      return "[Preset Speed 1] (FNPS1), Preset speed 1";
    case cslfn_e::fnps2:
      return "[Preset Speed 2] (FNPS2), Preset speed 2";
    case cslfn_e::fnpr1:
      return "[PID Ref Freq 1] (FNPR1), PID reference frequency 1";
    case cslfn_e::fnpr2:
      return "[PID Ref Freq 2] (FNPR2), PID reference frequency 2";
    case cslfn_e::fnusp:
      return "[+speed] (FNUSP), Increase speed";
    case cslfn_e::fndsp:
      return "[-speed] (FNDSP), Decrease speed";
  }
  return "unknown";
}
constexpr auto format_as(cslfn_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of cslout_e enum decleration
enum struct cslout_e : std::uint16_t {
  no = 0,      ///< No ([No] (NO))
  r2 = 146,    ///< Relay R2 ([R2] (R2))
  r3 = 147,    ///< Relay R3 ([R3] (R3))
  r4 = 148,    ///< Relay R4 ([R4] (R4))
  r5 = 149,    ///< Relay R5 ([R5] (R5))
  r6 = 150,    ///< Relay R6 ([R6] (R6))
  do11 = 163,  ///< DQ11 digital output  ([DQ11 Digital Output ] (DO11))
  do12 = 164,  ///< DQ12 digital output  ([DQ12 Digital Output ] (DO12))
  vsp = 200,   ///< VSP ([VSP] (VSP))
  r60 = 209,   ///< Relay R60 ([R60] (R60))
  r61 = 210,   ///< Relay R61 ([R61] (R61))
  r62 = 211,   ///< Relay R62 ([R62] (R62))
  r63 = 212,   ///< Relay R63 ([R63] (R63))
  r64 = 213,   ///< Relay R64 ([R64] (R64))
  r65 = 214,   ///< Relay R65 ([R65] (R65))
  r66 = 215,   ///< Relay R66 ([R66] (R66))
};
[[nodiscard]] constexpr auto enum_desc(cslout_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case cslout_e::no:
      return "[No] (NO), No";
    case cslout_e::r2:
      return "[R2] (R2), Relay R2";
    case cslout_e::r3:
      return "[R3] (R3), Relay R3";
    case cslout_e::r4:
      return "[R4] (R4), Relay R4";
    case cslout_e::r5:
      return "[R5] (R5), Relay R5";
    case cslout_e::r6:
      return "[R6] (R6), Relay R6";
    case cslout_e::do11:
      return "[DQ11 Digital Output ] (DO11), DQ11 digital output ";
    case cslout_e::do12:
      return "[DQ12 Digital Output ] (DO12), DQ12 digital output ";
    case cslout_e::vsp:
      return "[VSP] (VSP), VSP";
    case cslout_e::r60:
      return "[R60] (R60), Relay R60";
    case cslout_e::r61:
      return "[R61] (R61), Relay R61";
    case cslout_e::r62:
      return "[R62] (R62), Relay R62";
    case cslout_e::r63:
      return "[R63] (R63), Relay R63";
    case cslout_e::r64:
      return "[R64] (R64), Relay R64";
    case cslout_e::r65:
      return "[R65] (R65), Relay R65";
    case cslout_e::r66:
      return "[R66] (R66), Relay R66";
  }
  return "unknown";
}
constexpr auto format_as(cslout_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ctt_e enum decleration
enum struct ctt_e : std::uint16_t {
  std = 3,    ///< U/F VC Standard motor law ([U/F VC Standard] (STD))
  uf5 = 4,    ///< U/F VC 5 point voltage/frequency ([U/F VC 5pts] (UF5))
  ufq = 6,    ///< U/F VC Quadratic ([U/F VC Quad.] (UFQ))
  synu = 10,  ///< SYN_U VC law ([SYN_U VC] (SYNU))
  eco = 11,   ///< U/F VC Energy Sav. ([U/F VC Energy Sav.] (ECO))
  srvc = 12,  ///< Reluctance motor ([Reluctance Motor] (SRVC))
};
[[nodiscard]] constexpr auto enum_desc(ctt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ctt_e::std:
      return "[U/F VC Standard] (STD), U/F VC Standard motor law";
    case ctt_e::uf5:
      return "[U/F VC 5pts] (UF5), U/F VC 5 point voltage/frequency";
    case ctt_e::ufq:
      return "[U/F VC Quad.] (UFQ), U/F VC Quadratic";
    case ctt_e::synu:
      return "[SYN_U VC] (SYNU), SYN_U VC law";
    case ctt_e::eco:
      return "[U/F VC Energy Sav.] (ECO), U/F VC Energy Sav.";
    case ctt_e::srvc:
      return "[Reluctance Motor] (SRVC), Reluctance motor";
  }
  return "unknown";
}
constexpr auto format_as(ctt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of dcrc_e enum decleration
enum struct dcrc_e : std::uint16_t {
  no = 0,    ///< Ignore monitoring ([Ignore] (NO))
  warn = 1,  ///< Warning triggered ([Warning] (WARN))
  flt = 2,   ///< Error triggered ([Error] (FLT))
};
[[nodiscard]] constexpr auto enum_desc(dcrc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case dcrc_e::no:
      return "[Ignore] (NO), Ignore monitoring";
    case dcrc_e::warn:
      return "[Warning] (WARN), Warning triggered";
    case dcrc_e::flt:
      return "[Error] (FLT), Error triggered";
  }
  return "unknown";
}
constexpr auto format_as(dcrc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of dlr_e enum decleration
enum struct dlr_e : std::uint16_t {
  dlr0 = 0,  ///< Drive Locked ([Locked drv] (DLR0))
  dlr1 = 1,  ///< Drive unlocked ([Unlock. drv] (DLR1))
  dlr2 = 2,  ///< Download not allowed ([Not allowed] (DLR2))
  dlr3 = 3,  ///< Combined Lock/Unlock ([Lock/unlock] (DLR3))
};
[[nodiscard]] constexpr auto enum_desc(dlr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case dlr_e::dlr0:
      return "[Locked drv] (DLR0), Drive Locked";
    case dlr_e::dlr1:
      return "[Unlock. drv] (DLR1), Drive unlocked";
    case dlr_e::dlr2:
      return "[Not allowed] (DLR2), Download not allowed";
    case dlr_e::dlr3:
      return "[Lock/unlock] (DLR3), Combined Lock/Unlock";
  }
  return "unknown";
}
constexpr auto format_as(dlr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of dotd_e enum decleration
enum struct dotd_e : std::uint16_t {
  nst = 0,  ///< Drive freewheel stop ([Freewheel Stop] (NST))
  rmp = 1,  ///< Ramp stop ([Ramp Stop] (RMP))
};
[[nodiscard]] constexpr auto enum_desc(dotd_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case dotd_e::nst:
      return "[Freewheel Stop] (NST), Drive freewheel stop";
    case dotd_e::rmp:
      return "[Ramp Stop] (RMP), Ramp stop";
  }
  return "unknown";
}
constexpr auto format_as(dotd_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of dpma_e enum decleration
enum struct dpma_e : std::uint16_t {
  master_1 = 1,  ///< Master 1 ([Master 1] (1))
  master_2 = 2,  ///< Master 2 ([Master 2] (2))
};
[[nodiscard]] constexpr auto enum_desc(dpma_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case dpma_e::master_1:
      return "[Master 1] (1), Master 1";
    case dpma_e::master_2:
      return "[Master 2] (2), Master 2";
  }
  return "unknown";
}
constexpr auto format_as(dpma_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of drt_e enum decleration
enum struct drt_e : std::uint16_t {
  normal = 0,  ///< Normal duty ([Normal Duty] (NORMAL))
  high = 1,    ///< High duty ([Heavy Duty] (HIGH))
};
[[nodiscard]] constexpr auto enum_desc(drt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case drt_e::normal:
      return "[Normal Duty] (NORMAL), Normal duty";
    case drt_e::high:
      return "[Heavy Duty] (HIGH), High duty";
  }
  return "unknown";
}
constexpr auto format_as(drt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of drym_e enum decleration
enum struct drym_e : std::uint16_t {
  no = 0,   ///< No ([No] (NO))
  swt = 1,  ///< Switch ([Switch] (SWT))
  pwr = 2,  ///< Power ([Power] (PWR))
};
[[nodiscard]] constexpr auto enum_desc(drym_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case drym_e::no:
      return "[No] (NO), No";
    case drym_e::swt:
      return "[Switch] (SWT), Switch";
    case drym_e::pwr:
      return "[Power] (PWR), Power";
  }
  return "unknown";
}
constexpr auto format_as(drym_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of dur_e enum decleration
enum struct dur_e : std::uint16_t {
  minutes_5 = 0,   ///< 5 minutes ([5 minutes] (5))
  minutes_10 = 1,  ///< 10 minutes ([10 minutes] (10))
  minutes_30 = 2,  ///< 30 minutes ([30 minutes] (30))
  hours_1 = 3,     ///< 1 hour ([1 hour] (1H))
  hours_2 = 4,     ///< 2 hours ([2 hours] (2H))
  hours_3 = 5,     ///< 3 hours ([3 hours] (3H))
  ct = 6,          ///< Unlimited ([Unlimited] (CT))
};
[[nodiscard]] constexpr auto enum_desc(dur_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case dur_e::minutes_5:
      return "[5 minutes] (5), 5 minutes";
    case dur_e::minutes_10:
      return "[10 minutes] (10), 10 minutes";
    case dur_e::minutes_30:
      return "[30 minutes] (30), 30 minutes";
    case dur_e::hours_1:
      return "[1 hour] (1H), 1 hour";
    case dur_e::hours_2:
      return "[2 hours] (2H), 2 hours";
    case dur_e::hours_3:
      return "[3 hours] (3H), 3 hours";
    case dur_e::ct:
      return "[Unlimited] (CT), Unlimited";
  }
  return "unknown";
}
constexpr auto format_as(dur_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Torque or current limit stop
// Begin of ecfg_e enum decleration
enum struct ecfg_e : std::uint16_t {
  no = 0,   ///< Ignore ([Ignore] (NO))
  yes = 1,  ///< Freewheel stop ([Freewheel Stop] (YES))
  stt = 2,  ///< Per STT ([Per STT] (STT))
  lff = 4,  ///< Fallback speed ([Fallback Speed] (LFF))
  rls = 5,  ///< Speed maintained ([Speed maintained] (RLS))
  rmp = 6,  ///< Ramp stop ([Ramp stop] (RMP))
  fst = 7,  ///< Fast stop ([Fast stop] (FST))
  dci = 8,  ///< DC injection ([DC injection] (DCI))
};
[[nodiscard]] constexpr auto enum_desc(ecfg_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ecfg_e::no:
      return "[Ignore] (NO), Ignore";
    case ecfg_e::yes:
      return "[Freewheel Stop] (YES), Freewheel stop";
    case ecfg_e::stt:
      return "[Per STT] (STT), Per STT";
    case ecfg_e::lff:
      return "[Fallback Speed] (LFF), Fallback speed";
    case ecfg_e::rls:
      return "[Speed maintained] (RLS), Speed maintained";
    case ecfg_e::rmp:
      return "[Ramp stop] (RMP), Ramp stop";
    case ecfg_e::fst:
      return "[Fast stop] (FST), Fast stop";
    case ecfg_e::dci:
      return "[DC injection] (DCI), DC injection";
  }
  return "unknown";
}
constexpr auto format_as(ecfg_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of efdr_e enum decleration
enum struct efdr_e : std::uint16_t {
  no = 0,     ///< No error ([No Error] (NO))
  tout = 1,   ///< Server timeout ([Server Timeout] (TOUT))
  snf = 2,    ///< Server no file ([Server No File] (SNF))
  crpt = 3,   ///< Server corrupt file ([Server Corrupt File] (CRPT))
  epty = 4,   ///< Server empty file ([Server Empty File] (EPTY))
  hinv = 5,   ///< Drive invalid file ([Drive Invalid File] (HINV))
  crc = 6,    ///< CRC error ([CRC Error] (CRC))
  vrm = 7,    ///< Version incompatibility ([Version Incompatibility] (VRM))
  hnf = 9,    ///< Drive no file ([Drive No File] (HNF))
  size = 10,  ///< Server reading Size ([Server Reading Size] (SIZE))
  open = 11,  ///< Drive opening file ([Drive Opening File] (OPEN))
  read = 12,  ///< Drive reading file ([Drive Reading File] (READ))
  scnt = 13,  ///< Incompatibility ([Incompatibility] (SCNT))
  ninv = 14,  ///< Drive invalid name ([Drive Invalid Name] (NINV))
  fsiz = 15,  ///< Server incorrect file size ([Server Incorrect File Size] (FSIZ))
  hwf = 16,   ///< Drive writing file ([Drive Writing File] (HWF))
  swf = 17,   ///< Server writing file ([Server Writing File] (SWF))
};
[[nodiscard]] constexpr auto enum_desc(efdr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case efdr_e::no:
      return "[No Error] (NO), No error";
    case efdr_e::tout:
      return "[Server Timeout] (TOUT), Server timeout";
    case efdr_e::snf:
      return "[Server No File] (SNF), Server no file";
    case efdr_e::crpt:
      return "[Server Corrupt File] (CRPT), Server corrupt file";
    case efdr_e::epty:
      return "[Server Empty File] (EPTY), Server empty file";
    case efdr_e::hinv:
      return "[Drive Invalid File] (HINV), Drive invalid file";
    case efdr_e::crc:
      return "[CRC Error] (CRC), CRC error";
    case efdr_e::vrm:
      return "[Version Incompatibility] (VRM), Version incompatibility";
    case efdr_e::hnf:
      return "[Drive No File] (HNF), Drive no file";
    case efdr_e::size:
      return "[Server Reading Size] (SIZE), Server reading Size";
    case efdr_e::open:
      return "[Drive Opening File] (OPEN), Drive opening file";
    case efdr_e::read:
      return "[Drive Reading File] (READ), Drive reading file";
    case efdr_e::scnt:
      return "[Incompatibility] (SCNT), Incompatibility";
    case efdr_e::ninv:
      return "[Drive Invalid Name] (NINV), Drive invalid name";
    case efdr_e::fsiz:
      return "[Server Incorrect File Size] (FSIZ), Server incorrect file size";
    case efdr_e::hwf:
      return "[Drive Writing File] (HWF), Drive writing file";
    case efdr_e::swf:
      return "[Server Writing File] (SWF), Server writing file";
  }
  return "unknown";
}
constexpr auto format_as(efdr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of emdt_e enum decleration
enum struct emdt_e : std::uint16_t {
  kwct = 0,  ///< KW counter ([KW Counter] (KWCT))
  cve = 1,   ///< Instantaneous kW trend ([Instant. kW trend] (CVE))
  hsd = 2,   ///< Daily kWh report ([Daily kWh report] (HSD))
  hsw = 3,   ///< Weekly kWh report ([Weekly kWh report] (HSW))
  hsm = 4,   ///< Monthly kWh report ([Monthly kWh report] (HSM))
  hsy = 5,   ///< Yearly kWh report ([Yearly kWh report] (HSY))
};
[[nodiscard]] constexpr auto enum_desc(emdt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case emdt_e::kwct:
      return "[KW Counter] (KWCT), KW counter";
    case emdt_e::cve:
      return "[Instant. kW trend] (CVE), Instantaneous kW trend";
    case emdt_e::hsd:
      return "[Daily kWh report] (HSD), Daily kWh report";
    case emdt_e::hsw:
      return "[Weekly kWh report] (HSW), Weekly kWh report";
    case emdt_e::hsm:
      return "[Monthly kWh report] (HSM), Monthly kWh report";
    case emdt_e::hsy:
      return "[Yearly kWh report] (HSY), Yearly kWh report";
  }
  return "unknown";
}
constexpr auto format_as(emdt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// TODO: This enumeration seems wrong, there are multiple enums defined with the same value. fix if needed.
//  Begin of errd_e enum decleration
//  enum struct errd_e: std::uint16_t {
//     nof =                                 0, ///< No error detected ([No Error] (NOF))
//     crf1 =                                4096, ///< Precharge capacitor ([Precharge Capacitor] (CRF1))
//     sof =                                 4096, ///< Motor overspeed ([Motor Overspeed] (SOF))
//     infe =                                4096, ///< Internal error 14 (CPU) ([Internal Error 14] (INFE))
//     csf =                                 4096, ///< Channel switching detected error ([Channel Switch Error] (CSF))
//     asf =                                 4096, ///< Angle error ([Angle error] (ASF))
//     scf6 =                                8496, ///< AFE ShortCircuit error ([AFE ShortCircuit error] (SCF6))
//     scf4 =                                8752, ///< IGBT short circuit ([IGBT Short Circuit] (SCF4))
//     ocf =                                 8976, ///< Overcurrent ([Overcurrent] (OCF))
//     olc =                                 8977, ///< Process overload ([Process Overload] (OLC))
//     scf1 =                                8992, ///< Motor short circuit ([Motor short circuit] (SCF1))
//     scf5 =                                8992, ///< Motor short circuit ([Motor Short Circuit] (SCF5))
//     scf3 =                                9008, ///< Ground short circuit ([Ground Short Circuit] (SCF3))
//     olf =                                 9040, ///< Motor overload ([Motor Overload] (OLF))
//     osf =                                 12560, ///< Supply mains overvoltage ([Supply Mains Overvoltage] (OSF))
//     usf =                                 12576, ///< Supply mains undervoltage ([Supply Mains UnderV] (USF))
//     phf =                                 12592, ///< Input phase loss ([Input phase loss] (PHF))
//     obf2 =                                12928, ///< AFE Bus unbalancing  ([AFE Bus unbalancing ] (OBF2))
//     opf1 =                                13056, ///< Single output phase loss ([Single output phase loss] (OPF1))
//     obf =                                 13072, ///< DC bus overvoltage ([DC Bus Overvoltage] (OBF))
//     opf2 =                                13072, ///< Output phase loss ([Output Phase Loss] (OPF2))
//     ihf =                                 16912, ///< Input Overheating ([Input Overheating] (IHF))
//     ohf =                                 16912, ///< Drive overheating ([Drive Overheating] (OHF))
//     tjf =                                 16912, ///< IGBT overheating ([IGBT Overheating] (TJF))
//     ffdf =                                16912, ///< Fan feedback error ([Fan Feedback Error] (FFDF))
//     mof =                                 16912, ///< Module overheat error ([Module Overheat] (MOF))
//     th2f =                                16912, ///< AI2 thermal level error ([AI2 Th Level Error] (TH2F))
//     th3f =                                16912, ///< AI3 thermal level error ([AI3 Th Level Error] (TH3F))
//     th4f =                                16912, ///< AI4 thermal level error ([AI4 Th Level Error] (TH4F))
//     th5f =                                16912, ///< AI5 thermal level error ([AI5 Th Level Error] (TH5F))
//     fcf1 =                                20480, ///< Output contactor closed error ([Out Contact Closed Error] (FCF1))
//     fcf2 =                                20480, ///< Output contactor opened error ([Out Contact Opened Error] (FCF2))
//     lcf =                                 20480, ///< input contactor ([Input Contactor] (LCF))
//     idlf =                                20480, ///< Idle mode exit error ([Egy Saving Exit Error] (IDLF))
//     inf8 =                                20736, ///< Internal error 8 (Switching Supply) ([Internal Error 8] (INF8))
//     infa =                                20736, ///< Internal error 10 (Mains) ([Internal Error 10] (INFA))
//     inf9 =                                21008, ///< Internal error 9 (Measure) ([Internal Error 9] (INF9))
//     infb =                                21008, ///< Internal error 11 (Temperature) ([Internal Error 11] (INFB))
//     infc =                                21008, ///< Internal error 12 (Internal current supply) ([Internal Error 12]
//     (INFC)) infd =                                21008, ///< Internal error 13 (Diff current) ([Internal Error 13]
//     (INFD)) inff =                                21760, ///< Internal error 15 (Flash) ([Internal Error 15] (INFF)) eef1
//     =                                21808, ///< EEPROM control ([EEPROM Control] (EEF1)) eef2 = 21808, ///< EEPROM power
//     ([EEPROM Power] (EEF2)) ilf =                                 24832, ///< Internal communication interruption with
//     option module  ([Internal Link Error] (ILF)) inf1 =                                24832, ///< Internal error 1
//     (Rating) ([Internal Error 1] (INF1)) inf2 =                                24832, ///< Internal error 2 (Soft)
//     ([Internal Error 2] (INF2)) inf3 =                                24832, ///< Internal error 3 (Intern Comm)
//     ([Internal Error 3] (INF3)) inf4 =                                24832, ///< Internal error 4 (Manufacturing)
//     ([Internal Error 4] (INF4)) inf7 =                                24832, ///< Internal error 7 (Init) ([Internal Error
//     7] (INF7)) inf6 =                                24832, ///< Internal error 6 (Option) ([Internal Error 6] (INF6)) hcf
//     =                                 24832, ///< Boards compatibility ([Boards Compatibility] (HCF)) inf0 = 24832, ///<
//     Internal error 0 (IPC) ([Internal Error 0] (INF0)) infl =                                24832, ///< Internal error 21
//     (RTC) ([Internal Error 21] (INFL)) infm =                                24832, ///< Internal error 22 (Embedded
//     Ethernet) ([Internal Error 22] (INFM)) infp =                                24832, ///< Internal error 25
//     (Incompatibility CB & SW) ([Internal Error 25] (INFP)) infk =                                24832, ///< Internal
//     error 20 (option interface PCBA) ([Internal Error 20] (INFK)) infr =                                24832, ///<
//     Internal error 27 (Diagnostics CPLD) ([Internal Error 27] (INFR)) ifa =                                 24832, ///<
//     Monitoring circuit A error ([MonitorCircuit A Error] (IFA)) ifb =                                 24832, ///<
//     Monitoring circuit B error ([MonitorCircuit B Error] (IFB)) ifc =                                 24832, ///<
//     Monitoring circuit C error ([MonitorCircuit C Error] (IFC)) ifd =                                 24832, ///<
//     Monitoring circuit D error ([MonitorCircuit D Error] (IFD)) cfa =                                 24832, ///< Cabinet
//     circuit A error ([CabinetCircuit A Error] (CFA)) cfb =                                 24832, ///< Cabinet circuit B
//     error ([CabinetCircuit B Error] (CFB)) cfc =                                 24832, ///< Cabinet circuit C error
//     ([CabinetCircuit C Error] (CFC)) tfa =                                 24832, ///< Motor winding A error
//     ([MotorWinding A Error] (TFA)) tfb =                                 24832, ///< Motor winding B error ([MotorWinding
//     B Error] (TFB)) tfc =                                 24832, ///< Motor bearing A error ([MotorBearing A Error] (TFC))
//     tfd =                                 24832, ///< Motor bearing B error ([MotorBearing B Error] (TFD))
//     chf =                                 24832, ///< Cabinet overheat  error ([Cabinet Overheat  Error] (CHF))
//     urf =                                 24832, ///< AFE Mains undervoltage  ([AFE Mains Undervoltage ] (URF))
//     infv =                                24832, ///< Internal error 31 (Missing brick) ([Internal Error 31] (INFV))
//     inft =                                24832, ///< Internal error 29 (Inverter) ([Internal Error 29] (INFT))
//     infu =                                24832, ///< Internal error 30 (Rectifier) ([Internal Error 30] (INFU))
//     tjf2 =                                24832, ///< AFE IGBT over-heat error ([AFE IGBT over-heat error] (TJF2))
//     crf3 =                                24832, ///< AFE contactor feedback error ([AFE contactor feedback error] (CRF3))
//     acf1 =                                24832, ///< AFE modulation rate error ([AFE Modulation Rate Error] (ACF1))
//     acf2 =                                24832, ///< AFE current control error ([AFE Current Control Error] (ACF2))
//     mff =                                 24832, ///< Mains frequency out of range ([Mains Freq Out Of Range] (MFF))
//     cff =                                 25344, ///< Incorrect configuration ([Incorrect Configuration] (CFF))
//     cfi =                                 25344, ///< Invalid configuration ([Invalid Configuration] (CFI))
//     cfi2 =                                25344, ///< Configuration transfer error ([Conf Transfer Error] (CFI2))
//     cfi3 =                                25344, ///< Pre-settings transfer error ([Pre-settings Transfer Error] (CFI3))
//     cbf =                                 25344, ///< Circuit breaker error ([Circuit Breaker Error] (CBF))
//     infg =                                28672, ///< Internal error 16 (IO module - relay) ([Internal Error 16] (INFG))
//     infh =                                28672, ///< Internal error 17 (IO module - Standard) ([Internal Error 17]
//     (INFH)) fwer =                                28672, ///< Firmware Update error ([Firmware Update Error] (FWER)) stf =
//     28961, ///< Motor stall detected error ([Motor Stall Error] (STF)) lff2 =                                29440, ///<
//     AI2 4-20mA loss ([AI2 4-20mA loss] (LFF2)) lff3 =                                29440, ///< AI3 4-20mA loss ([AI3
//     4-20mA loss] (LFF3)) lff4 =                                29440, ///< AI4 4-20mA loss ([AI4 4-20mA loss] (LFF4)) lff5
//     =                                29440, ///< AI5 4-20 mA loss ([AI5 4-20 mA loss] (LFF5)) lff1 = 29440, ///< AI1 4-20
//     mA loss ([AI1 4-20 mA loss] (LFF1)) t2cf =                                29440, ///< Thermal sensor error on AI2
//     ([AI2 Thermal Sensor Error] (T2CF)) t3cf =                                29440, ///< Thermal sensor error on AI3
//     ([AI3 Thermal Sensor Error] (T3CF)) t4cf =                                29440, ///< Thermal sensor error on AI4
//     ([AI4 Thermal Sensor Error] (T4CF)) t5cf =                                29440, ///< Thermal sensor error on AI5
//     ([AI5 Thermal Sensor Error] (T5CF)) pglf =                                29696, ///< Program loading detected error
//     ([Program Loading Error] (PGLF)) pgrf =                                29696, ///< Program running detected error
//     ([Program Running Error] (PGRF)) pfmf =                                29952, ///< PID feedback detected error ([PID
//     Feedback Error] (PFMF)) slf1 =                                29968, ///< Modbus communication interruption ([Modbus
//     Com Interruption] (SLF1)) slf3 =                                29968, ///< HMI communication interruption ([HMI Com
//     Interruption] (SLF3)) cnf =                                 29984, ///< Fieldbus communication interruption ([Fieldbus
//     Com Interrupt] (CNF)) slf2 =                                30000, ///< PC communication interruption ([PC Com
//     Interruption] (SLF2)) ethf =                                30016, ///< Embedded Ethernet communication interruption
//     ([Embd Eth Com Interrupt] (ETHF)) spfc =                                30736, ///< Security files corrupt ([Security
//     Files Corrupt] (SPFC)) cof =                                 33024, ///< CANopen communication interruption ([CANopen
//     Com Interrupt] (COF)) infn =                                34816, ///< Internal error 23 (Module link) ([Internal
//     Error 23] (INFN)) epf1 =                                36864, ///< External detected error ([External Error] (EPF1))
//     epf2 =                                36864, ///< External error detected by Fieldbus ([Fieldbus Error] (EPF2))
//     fdr1 =                                36864, ///< FDR Eth embedded error ([FDR 1 Error] (FDR1))
//     fdr2 =                                36864, ///< FDR Eth module error ([FDR 2 Error] (FDR2))
//     tnf =                                 65280, ///< Autotuning detected error ([Autotuning Error] (TNF))
//     ulf =                                 65283, ///< Process Underload ([Process Underload] (ULF))
//     saff =                                65283, ///< Safety function detected error ([Safety Function Error] (SAFF))
//     pcpf =                                65297, ///< Pump cycle start error  ([PumpCycle start Error ] (PCPF))
//     ippf =                                65297, ///< Inlet pressure detected error ([Inlet Pressure Error] (IPPF))
//     plff =                                65297, ///< Pump low flow detected error ([Pump Low Flow Error] (PLFF))
//     jamf =                                65297, ///< Anti Jam detected error ([Anti Jam Error] (JAMF))
//     dryf =                                65297, ///< Dry run detected error ([Dry Run Error] (DRYF))
//     oplf =                                65298, ///< Outlet pressure low ([Out Pressure Low] (OPLF))
//     hfpf =                                65298, ///< High flow error ([High Flow Error] (HFPF))
//     ophf =                                65298, ///< Outlet pressure high ([Out Pressure High] (OPHF))
//     mplf =                                65535, ///< Lead pump not available ([Lead Pump Error] (MPLF))
//     lclf =                                65535, ///< Low level error ([Low Level Error] (LCLF))
//     lchf =                                65535, ///< High level error ([High Level Error] (LCHF))
//     infs =                                65535, ///< Internal error 28 (AFE) ([Internal Error 28] (INFS))
//     mdlf =                                65535, ///< MultiDrive Link error ([MultiDrive Link Error] (MDLF))
//     mpdf =                                65535, ///< Multipump device error ([M/P Device Error] (MPDF))
//     p24c =                                65535, ///< Cabinet I/O 24V missing error ([Cab I/O 24V Error] (P24C))
//     dcre =                                65535, ///< DC Bus ripple error ([DC Bus Ripple Error] (DCRE))
//  };
//  [[nodiscard]] constexpr auto enum_desc(errd_e const enum_value) -> std::string_view {
//     switch(enum_value)
//     {
//        case errd_e::nof:
//           return "[No Error] (NOF), No error detected";
//        case errd_e::crf1:
//           return "[Precharge Capacitor] (CRF1), Precharge capacitor";
//        case errd_e::sof:
//           return "[Motor Overspeed] (SOF), Motor overspeed";
//        case errd_e::infe:
//           return "[Internal Error 14] (INFE), Internal error 14 (CPU)";
//        case errd_e::csf:
//           return "[Channel Switch Error] (CSF), Channel switching detected error";
//        case errd_e::asf:
//           return "[Angle error] (ASF), Angle error";
//        case errd_e::scf6:
//           return "[AFE ShortCircuit error] (SCF6), AFE ShortCircuit error";
//        case errd_e::scf4:
//           return "[IGBT Short Circuit] (SCF4), IGBT short circuit";
//        case errd_e::ocf:
//           return "[Overcurrent] (OCF), Overcurrent";
//        case errd_e::olc:
//           return "[Process Overload] (OLC), Process overload";
//        case errd_e::scf1:
//           return "[Motor short circuit] (SCF1), Motor short circuit";
//        case errd_e::scf5:
//           return "[Motor Short Circuit] (SCF5), Motor short circuit";
//        case errd_e::scf3:
//           return "[Ground Short Circuit] (SCF3), Ground short circuit";
//        case errd_e::olf:
//           return "[Motor Overload] (OLF), Motor overload";
//        case errd_e::osf:
//           return "[Supply Mains Overvoltage] (OSF), Supply mains overvoltage";
//        case errd_e::usf:
//           return "[Supply Mains UnderV] (USF), Supply mains undervoltage";
//        case errd_e::phf:
//           return "[Input phase loss] (PHF), Input phase loss";
//        case errd_e::obf2:
//           return "[AFE Bus unbalancing ] (OBF2), AFE Bus unbalancing ";
//        case errd_e::opf1:
//           return "[Single output phase loss] (OPF1), Single output phase loss";
//        case errd_e::obf:
//           return "[DC Bus Overvoltage] (OBF), DC bus overvoltage";
//        case errd_e::opf2:
//           return "[Output Phase Loss] (OPF2), Output phase loss";
//        case errd_e::ihf:
//           return "[Input Overheating] (IHF), Input Overheating";
//        case errd_e::ohf:
//           return "[Drive Overheating] (OHF), Drive overheating";
//        case errd_e::tjf:
//           return "[IGBT Overheating] (TJF), IGBT overheating";
//        case errd_e::ffdf:
//           return "[Fan Feedback Error] (FFDF), Fan feedback error";
//        case errd_e::mof:
//           return "[Module Overheat] (MOF), Module overheat error";
//        case errd_e::th2f:
//           return "[AI2 Th Level Error] (TH2F), AI2 thermal level error";
//        case errd_e::th3f:
//           return "[AI3 Th Level Error] (TH3F), AI3 thermal level error";
//        case errd_e::th4f:
//           return "[AI4 Th Level Error] (TH4F), AI4 thermal level error";
//        case errd_e::th5f:
//           return "[AI5 Th Level Error] (TH5F), AI5 thermal level error";
//        case errd_e::fcf1:
//           return "[Out Contact Closed Error] (FCF1), Output contactor closed error";
//        case errd_e::fcf2:
//           return "[Out Contact Opened Error] (FCF2), Output contactor opened error";
//        case errd_e::lcf:
//           return "[Input Contactor] (LCF), input contactor";
//        case errd_e::idlf:
//           return "[Egy Saving Exit Error] (IDLF), Idle mode exit error";
//        case errd_e::inf8:
//           return "[Internal Error 8] (INF8), Internal error 8 (Switching Supply)";
//        case errd_e::infa:
//           return "[Internal Error 10] (INFA), Internal error 10 (Mains)";
//        case errd_e::inf9:
//           return "[Internal Error 9] (INF9), Internal error 9 (Measure)";
//        case errd_e::infb:
//           return "[Internal Error 11] (INFB), Internal error 11 (Temperature)";
//        case errd_e::infc:
//           return "[Internal Error 12] (INFC), Internal error 12 (Internal current supply)";
//        case errd_e::infd:
//           return "[Internal Error 13] (INFD), Internal error 13 (Diff current)";
//        case errd_e::inff:
//           return "[Internal Error 15] (INFF), Internal error 15 (Flash)";
//        case errd_e::eef1:
//           return "[EEPROM Control] (EEF1), EEPROM control";
//        case errd_e::eef2:
//           return "[EEPROM Power] (EEF2), EEPROM power";
//        case errd_e::ilf:
//           return "[Internal Link Error] (ILF), Internal communication interruption with option module ";
//        case errd_e::inf1:
//           return "[Internal Error 1] (INF1), Internal error 1 (Rating)";
//        case errd_e::inf2:
//           return "[Internal Error 2] (INF2), Internal error 2 (Soft)";
//        case errd_e::inf3:
//           return "[Internal Error 3] (INF3), Internal error 3 (Intern Comm)";
//        case errd_e::inf4:
//           return "[Internal Error 4] (INF4), Internal error 4 (Manufacturing)";
//        case errd_e::inf7:
//           return "[Internal Error 7] (INF7), Internal error 7 (Init)";
//        case errd_e::inf6:
//           return "[Internal Error 6] (INF6), Internal error 6 (Option)";
//        case errd_e::hcf:
//           return "[Boards Compatibility] (HCF), Boards compatibility";
//        case errd_e::inf0:
//           return "[Internal Error 0] (INF0), Internal error 0 (IPC)";
//        case errd_e::infl:
//           return "[Internal Error 21] (INFL), Internal error 21 (RTC)";
//        case errd_e::infm:
//           return "[Internal Error 22] (INFM), Internal error 22 (Embedded Ethernet)";
//        case errd_e::infp:
//           return "[Internal Error 25] (INFP), Internal error 25 (Incompatibility CB & SW)";
//        case errd_e::infk:
//           return "[Internal Error 20] (INFK), Internal error 20 (option interface PCBA)";
//        case errd_e::infr:
//           return "[Internal Error 27] (INFR), Internal error 27 (Diagnostics CPLD)";
//        case errd_e::ifa:
//           return "[MonitorCircuit A Error] (IFA), Monitoring circuit A error";
//        case errd_e::ifb:
//           return "[MonitorCircuit B Error] (IFB), Monitoring circuit B error";
//        case errd_e::ifc:
//           return "[MonitorCircuit C Error] (IFC), Monitoring circuit C error";
//        case errd_e::ifd:
//           return "[MonitorCircuit D Error] (IFD), Monitoring circuit D error";
//        case errd_e::cfa:
//           return "[CabinetCircuit A Error] (CFA), Cabinet circuit A error";
//        case errd_e::cfb:
//           return "[CabinetCircuit B Error] (CFB), Cabinet circuit B error";
//        case errd_e::cfc:
//           return "[CabinetCircuit C Error] (CFC), Cabinet circuit C error";
//        case errd_e::tfa:
//           return "[MotorWinding A Error] (TFA), Motor winding A error";
//        case errd_e::tfb:
//           return "[MotorWinding B Error] (TFB), Motor winding B error";
//        case errd_e::tfc:
//           return "[MotorBearing A Error] (TFC), Motor bearing A error";
//        case errd_e::tfd:
//           return "[MotorBearing B Error] (TFD), Motor bearing B error";
//        case errd_e::chf:
//           return "[Cabinet Overheat  Error] (CHF), Cabinet overheat  error";
//        case errd_e::urf:
//           return "[AFE Mains Undervoltage ] (URF), AFE Mains undervoltage ";
//        case errd_e::infv:
//           return "[Internal Error 31] (INFV), Internal error 31 (Missing brick)";
//        case errd_e::inft:
//           return "[Internal Error 29] (INFT), Internal error 29 (Inverter)";
//        case errd_e::infu:
//           return "[Internal Error 30] (INFU), Internal error 30 (Rectifier)";
//        case errd_e::tjf2:
//           return "[AFE IGBT over-heat error] (TJF2), AFE IGBT over-heat error";
//        case errd_e::crf3:
//           return "[AFE contactor feedback error] (CRF3), AFE contactor feedback error";
//        case errd_e::acf1:
//           return "[AFE Modulation Rate Error] (ACF1), AFE modulation rate error";
//        case errd_e::acf2:
//           return "[AFE Current Control Error] (ACF2), AFE current control error";
//        case errd_e::mff:
//           return "[Mains Freq Out Of Range] (MFF), Mains frequency out of range";
//        case errd_e::cff:
//           return "[Incorrect Configuration] (CFF), Incorrect configuration";
//        case errd_e::cfi:
//           return "[Invalid Configuration] (CFI), Invalid configuration";
//        case errd_e::cfi2:
//           return "[Conf Transfer Error] (CFI2), Configuration transfer error";
//        case errd_e::cfi3:
//           return "[Pre-settings Transfer Error] (CFI3), Pre-settings transfer error";
//        case errd_e::cbf:
//           return "[Circuit Breaker Error] (CBF), Circuit breaker error";
//        case errd_e::infg:
//           return "[Internal Error 16] (INFG), Internal error 16 (IO module - relay)";
//        case errd_e::infh:
//           return "[Internal Error 17] (INFH), Internal error 17 (IO module - Standard)";
//        case errd_e::fwer:
//           return "[Firmware Update Error] (FWER), Firmware Update error";
//        case errd_e::stf:
//           return "[Motor Stall Error] (STF), Motor stall detected error";
//        case errd_e::lff2:
//           return "[AI2 4-20mA loss] (LFF2), AI2 4-20mA loss";
//        case errd_e::lff3:
//           return "[AI3 4-20mA loss] (LFF3), AI3 4-20mA loss";
//        case errd_e::lff4:
//           return "[AI4 4-20mA loss] (LFF4), AI4 4-20mA loss";
//        case errd_e::lff5:
//           return "[AI5 4-20 mA loss] (LFF5), AI5 4-20 mA loss";
//        case errd_e::lff1:
//           return "[AI1 4-20 mA loss] (LFF1), AI1 4-20 mA loss";
//        case errd_e::t2cf:
//           return "[AI2 Thermal Sensor Error] (T2CF), Thermal sensor error on AI2";
//        case errd_e::t3cf:
//           return "[AI3 Thermal Sensor Error] (T3CF), Thermal sensor error on AI3";
//        case errd_e::t4cf:
//           return "[AI4 Thermal Sensor Error] (T4CF), Thermal sensor error on AI4";
//        case errd_e::t5cf:
//           return "[AI5 Thermal Sensor Error] (T5CF), Thermal sensor error on AI5";
//        case errd_e::pglf:
//           return "[Program Loading Error] (PGLF), Program loading detected error";
//        case errd_e::pgrf:
//           return "[Program Running Error] (PGRF), Program running detected error";
//        case errd_e::pfmf:
//           return "[PID Feedback Error] (PFMF), PID feedback detected error";
//        case errd_e::slf1:
//           return "[Modbus Com Interruption] (SLF1), Modbus communication interruption";
//        case errd_e::slf3:
//           return "[HMI Com Interruption] (SLF3), HMI communication interruption";
//        case errd_e::cnf:
//           return "[Fieldbus Com Interrupt] (CNF), Fieldbus communication interruption";
//        case errd_e::slf2:
//           return "[PC Com Interruption] (SLF2), PC communication interruption";
//        case errd_e::ethf:
//           return "[Embd Eth Com Interrupt] (ETHF), Embedded Ethernet communication interruption";
//        case errd_e::spfc:
//           return "[Security Files Corrupt] (SPFC), Security files corrupt";
//        case errd_e::cof:
//           return "[CANopen Com Interrupt] (COF), CANopen communication interruption";
//        case errd_e::infn:
//           return "[Internal Error 23] (INFN), Internal error 23 (Module link)";
//        case errd_e::epf1:
//           return "[External Error] (EPF1), External detected error";
//        case errd_e::epf2:
//           return "[Fieldbus Error] (EPF2), External error detected by Fieldbus";
//        case errd_e::fdr1:
//           return "[FDR 1 Error] (FDR1), FDR Eth embedded error";
//        case errd_e::fdr2:
//           return "[FDR 2 Error] (FDR2), FDR Eth module error";
//        case errd_e::tnf:
//           return "[Autotuning Error] (TNF), Autotuning detected error";
//        case errd_e::ulf:
//           return "[Process Underload] (ULF), Process Underload";
//        case errd_e::saff:
//           return "[Safety Function Error] (SAFF), Safety function detected error";
//        case errd_e::pcpf:
//           return "[PumpCycle start Error ] (PCPF), Pump cycle start error ";
//        case errd_e::ippf:
//           return "[Inlet Pressure Error] (IPPF), Inlet pressure detected error";
//        case errd_e::plff:
//           return "[Pump Low Flow Error] (PLFF), Pump low flow detected error";
//        case errd_e::jamf:
//           return "[Anti Jam Error] (JAMF), Anti Jam detected error";
//        case errd_e::dryf:
//           return "[Dry Run Error] (DRYF), Dry run detected error";
//        case errd_e::oplf:
//           return "[Out Pressure Low] (OPLF), Outlet pressure low";
//        case errd_e::hfpf:
//           return "[High Flow Error] (HFPF), High flow error";
//        case errd_e::ophf:
//           return "[Out Pressure High] (OPHF), Outlet pressure high";
//        case errd_e::mplf:
//           return "[Lead Pump Error] (MPLF), Lead pump not available";
//        case errd_e::lclf:
//           return "[Low Level Error] (LCLF), Low level error";
//        case errd_e::lchf:
//           return "[High Level Error] (LCHF), High level error";
//        case errd_e::infs:
//           return "[Internal Error 28] (INFS), Internal error 28 (AFE)";
//        case errd_e::mdlf:
//           return "[MultiDrive Link Error] (MDLF), MultiDrive Link error";
//        case errd_e::mpdf:
//           return "[M/P Device Error] (MPDF), Multipump device error";
//        case errd_e::p24c:
//           return "[Cab I/O 24V Error] (P24C), Cabinet I/O 24V missing error";
//        case errd_e::dcre:
//           return "[DC Bus Ripple Error] (DCRE), DC Bus ripple error";
//        default:
//           return "unknown";
//     }
//  }
//  constexpr auto format_as(errd_e const enum_value) -> std::string_view { return enum_desc(enum_value); }

// Begin of fcs_e enum decleration
enum struct fcs_e : std::uint16_t {
  no = 0,     ///< No ([No] (NO))
  rec0 = 1,   ///< Recall customer parameter set 0 ([Recall customer parameter set 0] (REC0))
  rec1 = 2,   ///< Recall customer parameter set 1 ([Recall customer parameter set 1] (REC1))
  rec2 = 3,   ///< Recall customer parameter set 2 ([Recall customer parameter set 2] (REC2))
  rec3 = 4,   ///< Recall customer parameter set 3 ([Recall customer parameter set 3] (REC3))
  ini = 64,   ///< Recall default parameter set ([Recall default parameter set] (INI))
  ini1 = 71,  ///< Recall OEM default parameter set ([Recall OEM default parameter set] (INI1))
};
[[nodiscard]] constexpr auto enum_desc(fcs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case fcs_e::no:
      return "[No] (NO), No";
    case fcs_e::rec0:
      return "[Recall customer parameter set 0] (REC0), Recall customer parameter set 0";
    case fcs_e::rec1:
      return "[Recall customer parameter set 1] (REC1), Recall customer parameter set 1";
    case fcs_e::rec2:
      return "[Recall customer parameter set 2] (REC2), Recall customer parameter set 2";
    case fcs_e::rec3:
      return "[Recall customer parameter set 3] (REC3), Recall customer parameter set 3";
    case fcs_e::ini:
      return "[Recall default parameter set] (INI), Recall default parameter set";
    case fcs_e::ini1:
      return "[Recall OEM default parameter set] (INI1), Recall OEM default parameter set";
  }
  return "unknown";
}
constexpr auto format_as(fcs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of fcsi_e enum decleration
enum struct fcsi_e : std::uint16_t {
  ini = 0,   ///< Macro configuration ([Macro Config] (INI))
  cfg1 = 2,  ///< Configuration 1 ([Config 1] (CFG1))
  cfg2 = 3,  ///< Configuration 2 ([Config 2] (CFG2))
  cfg3 = 4,  ///< Config 3 ([Config 3] (CFG3))
};
[[nodiscard]] constexpr auto enum_desc(fcsi_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case fcsi_e::ini:
      return "[Macro Config] (INI), Macro configuration";
    case fcsi_e::cfg1:
      return "[Config 1] (CFG1), Configuration 1";
    case fcsi_e::cfg2:
      return "[Config 2] (CFG2), Configuration 2";
    case fcsi_e::cfg3:
      return "[Config 3] (CFG3), Config 3";
  }
  return "unknown";
}
constexpr auto format_as(fcsi_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of fdra_e enum decleration
enum struct fdra_e : std::uint16_t {
  idle = 0,  ///< NOT ACTIVE ([NOT ACTIVE] (IDLE))
  save = 1,  ///< SAVE ([SAVE] (SAVE))
  rest = 2,  ///< REST ([REST] (REST))
};
[[nodiscard]] constexpr auto enum_desc(fdra_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case fdra_e::idle:
      return "[NOT ACTIVE] (IDLE), NOT ACTIVE";
    case fdra_e::save:
      return "[SAVE] (SAVE), SAVE";
    case fdra_e::rest:
      return "[REST] (REST), REST";
  }
  return "unknown";
}
constexpr auto format_as(fdra_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of fem_e enum decleration
enum struct fem_e : std::uint16_t {
  no = 0,  ///< Disable pump characteristics ([No] (NO))
  hq = 1,  ///< Enable Head vs flow curve ([HQ] (HQ))
  pq = 2,  ///< Activate power vs flow curve ([PQ] (PQ))
};
[[nodiscard]] constexpr auto enum_desc(fem_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case fem_e::no:
      return "[No] (NO), Disable pump characteristics";
    case fem_e::hq:
      return "[HQ] (HQ), Enable Head vs flow curve";
    case fem_e::pq:
      return "[PQ] (PQ), Activate power vs flow curve";
  }
  return "unknown";
}
constexpr auto format_as(fem_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ffm_e enum decleration
enum struct ffm_e : std::uint16_t {
  std = 0,  ///< Standard ([Standard] (STD))
  run = 1,  ///< Always ([Always] (RUN))
  stp = 2,  ///< Never ([Never] (STP))
  eco = 3,  ///< Economy ([Economy] (ECO))
};
[[nodiscard]] constexpr auto enum_desc(ffm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ffm_e::std:
      return "[Standard] (STD), Standard";
    case ffm_e::run:
      return "[Always] (RUN), Always";
    case ffm_e::stp:
      return "[Never] (STP), Never";
    case ffm_e::eco:
      return "[Economy] (ECO), Economy";
  }
  return "unknown";
}
constexpr auto format_as(ffm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of flcm_e enum decleration
enum struct flcm_e : std::uint16_t {
  no = 0,    ///< Inactive ([Inactive] (NO))
  mon = 1,   ///< Display ([Display] (MON))
  comp = 2,  ///< Compensation ([Compensation] (COMP))
};
[[nodiscard]] constexpr auto enum_desc(flcm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case flcm_e::no:
      return "[Inactive] (NO), Inactive";
    case flcm_e::mon:
      return "[Display] (MON), Display";
    case flcm_e::comp:
      return "[Compensation] (COMP), Compensation";
  }
  return "unknown";
}
constexpr auto format_as(flcm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of flr_e enum decleration
enum struct flr_e : std::uint16_t {
  no = 0,   ///< Not configured ([Not Configured] (NO))
  yes = 1,  ///< Yes on freewheel ([Yes On Freewheel] (YES))
  all = 2,  ///< Yes always ([Yes Always] (ALL))
};
[[nodiscard]] constexpr auto enum_desc(flr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case flr_e::no:
      return "[Not Configured] (NO), Not configured";
    case flr_e::yes:
      return "[Yes On Freewheel] (YES), Yes on freewheel";
    case flr_e::all:
      return "[Yes Always] (ALL), Yes always";
  }
  return "unknown";
}
constexpr auto format_as(flr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of for_e enum decleration
enum struct for_e : std::uint16_t {
  bits8_odd_1stop = 2,   ///< 8bit odd parity 1stop bit ([8-O-1] (8O1))
  bits8_even_1stop = 3,  ///< 8 bits even parity 1 stop bit ([8-E-1] (8E1))
  bits8_no_1stop = 4,    ///< 8bit no parity 1stop bit ([8-N-1] (8N1))
  bits8_no_2stop = 5,    ///< 8bit no parity 2stop bits ([8-N-2] (8N2))
};
[[nodiscard]] constexpr auto enum_desc(for_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case for_e::bits8_odd_1stop:
      return "[8-O-1] (8O1), 8bit odd parity 1stop bit";
    case for_e::bits8_even_1stop:
      return "[8-E-1] (8E1), 8 bits even parity 1 stop bit";
    case for_e::bits8_no_1stop:
      return "[8-N-1] (8N1), 8bit no parity 1stop bit";
    case for_e::bits8_no_2stop:
      return "[8-N-2] (8N2), 8bit no parity 2stop bits";
  }
  return "unknown";
}
constexpr auto format_as(for_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of fwer_e enum decleration
enum struct fwer_e : std::uint16_t {
  no = 0,      ///< No error ([No Error] (NO))
  lock = 1,    ///< Lock error ([Lock Error] (LOCK))
  md5 = 2,     ///< Package error ([Package Error] (MD5))
  comp = 3,    ///< Package compatibility error ([Package compatibility error] (COMP))
  ask = 4,     ///< Ask error ([Ask error] (ASK))
  reset = 5,   ///< Reset drive error ([Reset Drive Error] (RESET))
  save = 6,    ///< Configuration saving warning ([Conf Saving Warning] (SAVE))
  load = 7,    ///< Conf loading  warning ([Conf Loading  Warning] (LOAD))
  scp = 8,     ///< Post Script warning ([Post Script Warning] (SCP))
  des = 9,     ///< Package Description error ([Package Description Error] (DES))
  pkg = 10,    ///< Package not found ([Package not found] (PKG))
  spwr = 11,   ///< Power Supply error ([Power Supply error] (SPWR))
  btm3 = 12,   ///< Boot M3 error ([Boot M3 error] (BTM3))
  btc28 = 13,  ///< Boot C28 error ([Boot C28 error] (BTC28))
  m3 = 14,     ///< M3 error ([M3 Error] (M3))
  c28 = 15,    ///< C28 error ([C28 error] (C28))
  cpld = 16,   ///< CPLD error ([CPLD error] (CPLD))
  pwr = 17,    ///< Boot power error ([Boot Power Error] (PWR))
  embt = 18,   ///< Embedded ethernet boot error ([Emb. Eth Boot Error] (EMBT))
  emil = 19,   ///< Embedded ethernet error ([Emb. Eth Error] (EMIL))
  emwb = 20,   ///< Embedded ethernet WebServer error ([Emb. Eth Web Error] (EMWB))
  optbt = 21,  ///< Module ethernet boot error ([Module Eth Boot Error] (OPTBT))
  optil = 22,  ///< Module ethernet error ([Module Eth Error] (OPTIL))
  optwb = 23,  ///< Module ethernet WebServer error ([Module Eth Web Error] (OPTWB))
  pswd = 24,   ///< Password enabled ([Password enabled] (PSWD))
  mem = 25,    ///< Flash error ([Flash Error] (MEM))
  ifo = 26,    ///< Package error ([Package error] (IFO))
  wait = 27,   ///< Wait ([Wait] (WAIT))
};
[[nodiscard]] constexpr auto enum_desc(fwer_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case fwer_e::no:
      return "[No Error] (NO), No error";
    case fwer_e::lock:
      return "[Lock Error] (LOCK), Lock error";
    case fwer_e::md5:
      return "[Package Error] (MD5), Package error";
    case fwer_e::comp:
      return "[Package compatibility error] (COMP), Package compatibility error";
    case fwer_e::ask:
      return "[Ask error] (ASK), Ask error";
    case fwer_e::reset:
      return "[Reset Drive Error] (RESET), Reset drive error";
    case fwer_e::save:
      return "[Conf Saving Warning] (SAVE), Configuration saving warning";
    case fwer_e::load:
      return "[Conf Loading  Warning] (LOAD), Conf loading  warning";
    case fwer_e::scp:
      return "[Post Script Warning] (SCP), Post Script warning";
    case fwer_e::des:
      return "[Package Description Error] (DES), Package Description error";
    case fwer_e::pkg:
      return "[Package not found] (PKG), Package not found";
    case fwer_e::spwr:
      return "[Power Supply error] (SPWR), Power Supply error";
    case fwer_e::btm3:
      return "[Boot M3 error] (BTM3), Boot M3 error";
    case fwer_e::btc28:
      return "[Boot C28 error] (BTC28), Boot C28 error";
    case fwer_e::m3:
      return "[M3 Error] (M3), M3 error";
    case fwer_e::c28:
      return "[C28 error] (C28), C28 error";
    case fwer_e::cpld:
      return "[CPLD error] (CPLD), CPLD error";
    case fwer_e::pwr:
      return "[Boot Power Error] (PWR), Boot power error";
    case fwer_e::embt:
      return "[Emb. Eth Boot Error] (EMBT), Embedded ethernet boot error";
    case fwer_e::emil:
      return "[Emb. Eth Error] (EMIL), Embedded ethernet error";
    case fwer_e::emwb:
      return "[Emb. Eth Web Error] (EMWB), Embedded ethernet WebServer error";
    case fwer_e::optbt:
      return "[Module Eth Boot Error] (OPTBT), Module ethernet boot error";
    case fwer_e::optil:
      return "[Module Eth Error] (OPTIL), Module ethernet error";
    case fwer_e::optwb:
      return "[Module Eth Web Error] (OPTWB), Module ethernet WebServer error";
    case fwer_e::pswd:
      return "[Password enabled] (PSWD), Password enabled";
    case fwer_e::mem:
      return "[Flash Error] (MEM), Flash error";
    case fwer_e::ifo:
      return "[Package error] (IFO), Package error";
    case fwer_e::wait:
      return "[Wait] (WAIT), Wait";
  }
  return "unknown";
}
constexpr auto format_as(fwer_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of fwst_e enum decleration
enum struct fwst_e : std::uint16_t {
  check = 0,   ///< Firmware update  inactive ([Inactive] (CHECK))
  power = 1,   ///< Power update in progress ([PwrUpd in progress] (POWER))
  pend = 2,    ///< Power update pending ([PwrUpd Pending] (PEND))
  rdy = 3,     ///< Firmware update ready ([Ready] (RDY))
  no = 4,      ///< Firmware update  inactive ([Inactive] (NO))
  succd = 5,   ///< Firmware update succeded ([Succeeded] (SUCCD))
  failed = 6,  ///< Update error ([Update Error] (FAILED))
  prog = 7,    ///< Firmware update in progress ([In Progress] (PROG))
  rqstd = 8,   ///< Firmware update requested ([Requested] (RQSTD))
  trld = 9,    ///< Transfer In Progress ([Transfer In Progress] (TRLD))
  trok = 10,   ///< Transfer Done ([Transfer Done] (TROK))
  clear = 11,  ///< Package cleared ([Package cleared] (CLEAR))
  sucwr = 12,  ///< Firmware update succeded with warnings ([Warning] (SUCWR))
  flsta = 13,  ///< Drive State error ([Drive State Error] (FLSTA))
  flpkg = 14,  ///< Package error ([Package Error] (FLPKG))
  save = 15,   ///< Saving current configuration ([Saving conf] (SAVE))
  post = 16,   ///< Post Script ([Post Script] (POST))
};
[[nodiscard]] constexpr auto enum_desc(fwst_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case fwst_e::check:
      return "[Inactive] (CHECK), Firmware update  inactive";
    case fwst_e::power:
      return "[PwrUpd in progress] (POWER), Power update in progress";
    case fwst_e::pend:
      return "[PwrUpd Pending] (PEND), Power update pending";
    case fwst_e::rdy:
      return "[Ready] (RDY), Firmware update ready";
    case fwst_e::no:
      return "[Inactive] (NO), Firmware update  inactive";
    case fwst_e::succd:
      return "[Succeeded] (SUCCD), Firmware update succeded";
    case fwst_e::failed:
      return "[Update Error] (FAILED), Update error";
    case fwst_e::prog:
      return "[In Progress] (PROG), Firmware update in progress";
    case fwst_e::rqstd:
      return "[Requested] (RQSTD), Firmware update requested";
    case fwst_e::trld:
      return "[Transfer In Progress] (TRLD), Transfer In Progress";
    case fwst_e::trok:
      return "[Transfer Done] (TROK), Transfer Done";
    case fwst_e::clear:
      return "[Package cleared] (CLEAR), Package cleared";
    case fwst_e::sucwr:
      return "[Warning] (SUCWR), Firmware update succeded with warnings";
    case fwst_e::flsta:
      return "[Drive State Error] (FLSTA), Drive State error";
    case fwst_e::flpkg:
      return "[Package Error] (FLPKG), Package error";
    case fwst_e::save:
      return "[Saving conf] (SAVE), Saving current configuration";
    case fwst_e::post:
      return "[Post Script] (POST), Post Script";
  }
  return "unknown";
}
constexpr auto format_as(fwst_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of hmis_e enum decleration
enum struct hmis_e : std::uint16_t {
  tun = 0,     ///< Drive automatic tuning ([Autotuning] (TUN))
  dcb = 1,     ///< DC injection ([In DC inject.] (DCB))
  rdy = 2,     ///< Ready ([Ready] (RDY))
  nst = 3,     ///< Freewheel stop ([Freewheel] (NST))
  run = 4,     ///< Running ([Running] (RUN))
  acc = 5,     ///< Accelerating ([Accelerating] (ACC))
  dec = 6,     ///< Decelerating ([Decelerating] (DEC))
  cli = 7,     ///< In current limitation ([Current limitation] (CLI))
  fst = 8,     ///< Fast stop ([Fast stop] (FST))
  nlp = 11,    ///< No Mains voltage ([No Mains Voltage] (NLP))
  ctl = 13,    ///< Control stopping ([control.stop] (CTL))
  obr = 14,    ///< Dec ramp adaptation ([Dec. adapt.] (OBR))
  soc = 15,    ///< Output cut ([Output cut] (SOC))
  usa = 17,    ///< Undervoltage warning ([Undervoltage Warning] (USA))
  tc = 18,     ///< TC mode active ([TC Mode Active] (TC))
  st = 19,     ///< In autotest ([In autotest] (ST))
  fa = 20,     ///< Autotest error ([Autotest error] (FA))
  ok = 21,     ///< Autotest OK ([Autotest OK] (OK))
  ep = 22,     ///< EEprom test ([EEprom test] (EP))
  fault = 23,  ///< Operating state "Fault" ([Operating State "Fault" ] (FLT))
  dcp = 25,    ///< DCP Flashing mode ([DCP Flashing Mode] (DCP))
  sto = 30,    ///< STO active ([STO active] (STO))
  idle = 35,   ///< Energy Saving ([Energy Saving] (IDLE))
  fwup = 36,   ///< Firmware Update ([Firmware Update] (FWUP))
  ura = 37,    ///< AFE Mains undervoltage  ([AFE Mains Undervoltage ] (URA))
};
[[nodiscard]] constexpr auto enum_desc(hmis_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case hmis_e::tun:
      return "[Autotuning] (TUN), Drive automatic tuning";
    case hmis_e::dcb:
      return "[In DC inject.] (DCB), DC injection";
    case hmis_e::rdy:
      return "[Ready] (RDY), Ready";
    case hmis_e::nst:
      return "[Freewheel] (NST), Freewheel stop";
    case hmis_e::run:
      return "[Running] (RUN), Running";
    case hmis_e::acc:
      return "[Accelerating] (ACC), Accelerating";
    case hmis_e::dec:
      return "[Decelerating] (DEC), Decelerating";
    case hmis_e::cli:
      return "[Current limitation] (CLI), In current limitation";
    case hmis_e::fst:
      return "[Fast stop] (FST), Fast stop";
    case hmis_e::nlp:
      return "[No Mains Voltage] (NLP), No Mains voltage";
    case hmis_e::ctl:
      return "[control.stop] (CTL), Control stopping";
    case hmis_e::obr:
      return "[Dec. adapt.] (OBR), Dec ramp adaptation";
    case hmis_e::soc:
      return "[Output cut] (SOC), Output cut";
    case hmis_e::usa:
      return "[Undervoltage Warning] (USA), Undervoltage warning";
    case hmis_e::tc:
      return "[TC Mode Active] (TC), TC mode active";
    case hmis_e::st:
      return "[In autotest] (ST), In autotest";
    case hmis_e::fa:
      return "[Autotest error] (FA), Autotest error";
    case hmis_e::ok:
      return "[Autotest OK] (OK), Autotest OK";
    case hmis_e::ep:
      return "[EEprom test] (EP), EEprom test";
    case hmis_e::fault:
      return "[Operating State Fault ] (FLT), Operating state Fault";
    case hmis_e::dcp:
      return "[DCP Flashing Mode] (DCP), DCP Flashing mode";
    case hmis_e::sto:
      return "[STO active] (STO), STO active";
    case hmis_e::idle:
      return "[Energy Saving] (IDLE), Energy Saving";
    case hmis_e::fwup:
      return "[Firmware Update] (FWUP), Firmware Update";
    case hmis_e::ura:
      return "[AFE Mains Undervoltage ] (URA), AFE Mains undervoltage ";
  }
  return "unknown";
}
constexpr auto format_as(hmis_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ifm_e enum decleration
enum struct ifm_e : std::uint16_t {
  all = 0,  ///< Always active ([Always Active] (ALL))
  rry = 1,  ///< Ready and Run state ([Ready & Run State] (RRY))
  run = 2,  ///< Run state ([Run State] (RUN))
};
[[nodiscard]] constexpr auto enum_desc(ifm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ifm_e::all:
      return "[Always Active] (ALL), Always active";
    case ifm_e::rry:
      return "[Ready & Run State] (RRY), Ready and Run state";
    case ifm_e::run:
      return "[Run State] (RUN), Run state";
  }
  return "unknown";
}
constexpr auto format_as(ifm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of inhs_e enum decleration
enum struct inhs_e : std::uint16_t {
  no = 0,   ///< Disabled ([Disabled] (NO))
  frd = 1,  ///< Forced Run in forward direction ([Forced Run FW] (FRD))
  rrs = 2,  ///< Forced  Run in reverse direction ([Forced Run RV] (RRS))
};
[[nodiscard]] constexpr auto enum_desc(inhs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case inhs_e::no:
      return "[Disabled] (NO), Disabled";
    case inhs_e::frd:
      return "[Forced Run FW] (FRD), Forced Run in forward direction";
    case inhs_e::rrs:
      return "[Forced Run RV] (RRS), Forced  Run in reverse direction";
  }
  return "unknown";
}
constexpr auto format_as(inhs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of inr_e enum decleration
enum struct inr_e : std::uint16_t {
  hundredth_of_a_second = 0,  ///< hundredths of seconds ([0.01] (001))
  tenth_of_a_second = 1,      ///< Tenths of seconds ([0.1] (01))
  second = 2,                 ///< seconds ([1] (1))
};
[[nodiscard]] constexpr auto enum_desc(inr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case inr_e::hundredth_of_a_second:
      return "[0.01] (001), hundredths of seconds";
    case inr_e::tenth_of_a_second:
      return "[0.1] (01), Tenths of seconds";
    case inr_e::second:
      return "[1] (1), seconds";
  }
  return "unknown";
}
constexpr auto format_as(inr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ipae_e enum decleration
enum struct ipae_e : std::uint16_t {
  idle = 0,  ///< Idle State ([Idle State] (IDLE))
  init = 1,  ///< Init ([Init] (INIT))
  conf = 2,  ///< Configuration ([Configuration] (CONF))
  rdy = 3,   ///< Ready ([Ready] (RDY))
  ope = 4,   ///< Operational ([Operational] (OPE))
  ucfg = 5,  ///< Not Configured ([Not Configured] (UCFG))
  urec = 6,  ///< Unrecoverable error ([Unrecoverable Error] (UREC))
};
[[nodiscard]] constexpr auto enum_desc(ipae_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ipae_e::idle:
      return "[Idle State] (IDLE), Idle State";
    case ipae_e::init:
      return "[Init] (INIT), Init";
    case ipae_e::conf:
      return "[Configuration] (CONF), Configuration";
    case ipae_e::rdy:
      return "[Ready] (RDY), Ready";
    case ipae_e::ope:
      return "[Operational] (OPE), Operational";
    case ipae_e::ucfg:
      return "[Not Configured] (UCFG), Not Configured";
    case ipae_e::urec:
      return "[Unrecoverable Error] (UREC), Unrecoverable error";
  }
  return "unknown";
}
constexpr auto format_as(ipae_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ipm_e enum decleration
enum struct ipm_e : std::uint16_t {
  manu = 0,   ///< Fixed address ([Fixed] (MANU))
  bootp = 1,  ///< BOOTP ([BOOTP] (BOOTP))
  dhcp = 2,   ///< DHCP ([DHCP] (DHCP))
  dcp = 3,    ///< DCP ([DCP] (DCP))
};
[[nodiscard]] constexpr auto enum_desc(ipm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ipm_e::manu:
      return "[Fixed] (MANU), Fixed address";
    case ipm_e::bootp:
      return "[BOOTP] (BOOTP), BOOTP";
    case ipm_e::dhcp:
      return "[DHCP] (DHCP), DHCP";
    case ipm_e::dcp:
      return "[DCP] (DCP), DCP";
  }
  return "unknown";
}
constexpr auto format_as(ipm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ippm_e enum decleration
enum struct ippm_e : std::uint16_t {
  no = 0,     ///< NO ([No] (NO))
  alarm = 1,  ///< warning ([Warning] (ALARM))
  comp = 2,   ///< Compensation ([Compensation] (COMP))
};
[[nodiscard]] constexpr auto enum_desc(ippm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ippm_e::no:
      return "[No] (NO), NO";
    case ippm_e::alarm:
      return "[Warning] (ALARM), warning";
    case ippm_e::comp:
      return "[Compensation] (COMP), Compensation";
  }
  return "unknown";
}
constexpr auto format_as(ippm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of jatc_e enum decleration
enum struct jatc_e : std::uint16_t {
  no = 0,      ///< No ([No] (NO))
  start = 1,   ///< Start ([Start] (START))
  time = 2,    ///< Time ([Time] (TIME))
  torque = 3,  ///< Torque ([Torque] (TORQUE))
};
[[nodiscard]] constexpr auto enum_desc(jatc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case jatc_e::no:
      return "[No] (NO), No";
    case jatc_e::start:
      return "[Start] (START), Start";
    case jatc_e::time:
      return "[Time] (TIME), Time";
    case jatc_e::torque:
      return "[Torque] (TORQUE), Torque";
  }
  return "unknown";
}
constexpr auto format_as(jatc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lac_e enum decleration
enum struct lac_e : std::uint16_t {
  bas = 0,  ///< Basic access ([Basic] (BAS))
  std = 1,  ///< Standard access ([Standard] (STD))
  epr = 3,  ///< Expert access ([Expert] (EPR))
  ser = 4,  ///< Services ([Services] (SER))
};
[[nodiscard]] constexpr auto enum_desc(lac_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lac_e::bas:
      return "[Basic] (BAS), Basic access";
    case lac_e::std:
      return "[Standard] (STD), Standard access";
    case lac_e::epr:
      return "[Expert] (EPR), Expert access";
    case lac_e::ser:
      return "[Services] (SER), Services";
  }
  return "unknown";
}
constexpr auto format_as(lac_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lcm_e enum decleration
enum struct lcm_e : std::uint16_t {
  no = 0,     ///< Deactivated ([No] (NO))
  fill = 1,   ///< Filling mode ([Filling] (FILL))
  empty = 2,  ///< Emptying mode ([Emptying] (EMPTY))
};
[[nodiscard]] constexpr auto enum_desc(lcm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lcm_e::no:
      return "[No] (NO), Deactivated";
    case lcm_e::fill:
      return "[Filling] (FILL), Filling mode";
    case lcm_e::empty:
      return "[Emptying] (EMPTY), Emptying mode";
  }
  return "unknown";
}
constexpr auto format_as(lcm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lcnt_e enum decleration
enum struct lcnt_e : std::uint16_t {
  sw = 0,     ///< Level switches ([Level Switches] (SW))
  level = 1,  ///< Level sensor ([Level Sensor] (LEVEL))
  pres = 2,   ///< Pressure sensor ([Pressure sensor] (PRES))
};
[[nodiscard]] constexpr auto enum_desc(lcnt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lcnt_e::sw:
      return "[Level Switches] (SW), Level switches";
    case lcnt_e::level:
      return "[Level Sensor] (LEVEL), Level sensor";
    case lcnt_e::pres:
      return "[Pressure sensor] (PRES), Pressure sensor";
  }
  return "unknown";
}
constexpr auto format_as(lcnt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lcpm_e enum decleration
enum struct lcpm_e : std::uint16_t {
  comm = 0,   ///< All pumps stopped simultaneously ([Simultaneous Stop] (COMM))
  indiv = 1,  ///< Each pump stopped individually ([Individual Stop] (INDIV))
};
[[nodiscard]] constexpr auto enum_desc(lcpm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lcpm_e::comm:
      return "[Simultaneous Stop] (COMM), All pumps stopped simultaneously";
    case lcpm_e::indiv:
      return "[Individual Stop] (INDIV), Each pump stopped individually";
  }
  return "unknown";
}
constexpr auto format_as(lcpm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lcs_e enum decleration
enum struct lcs_e : std::uint16_t {
  none = 0,   ///< Not configured ([None] (NONE))
  nact = 1,   ///< Inactive ([Inactive] (NACT))
  fill = 2,   ///< Filling in progress ([Filling] (FILL))
  empty = 3,  ///< Emptying in progress ([Emptying] (EMPTY))
  low = 4,    ///< Low level ([Low Level] (LOW))
  high = 5,   ///< High level ([High Level] (HIGH))
};
[[nodiscard]] constexpr auto enum_desc(lcs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lcs_e::none:
      return "[None] (NONE), Not configured";
    case lcs_e::nact:
      return "[Inactive] (NACT), Inactive";
    case lcs_e::fill:
      return "[Filling] (FILL), Filling in progress";
    case lcs_e::empty:
      return "[Emptying] (EMPTY), Emptying in progress";
    case lcs_e::low:
      return "[Low Level] (LOW), Low level";
    case lcs_e::high:
      return "[High Level] (HIGH), High level";
  }
  return "unknown";
}
constexpr auto format_as(lcs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lcst_e enum decleration
enum struct lcst_e : std::uint16_t {
  trad = 0,   ///< Switches ([Switches] (TRAD))
  basic = 1,  ///< Standard ([Standard] (BASIC))
  adv = 2,    ///< Energy Optimized ([Energy Optimized] (ADV))
};
[[nodiscard]] constexpr auto enum_desc(lcst_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lcst_e::trad:
      return "[Switches] (TRAD), Switches";
    case lcst_e::basic:
      return "[Standard] (BASIC), Standard";
    case lcst_e::adv:
      return "[Energy Optimized] (ADV), Energy Optimized";
  }
  return "unknown";
}
constexpr auto format_as(lcst_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ldd_e enum decleration
enum struct ldd_e : std::uint16_t {
  no = 0,     ///< Distribution logging disable ([Distrib. Log. DISABLE] (NO))
  rfr = 1,    ///< Motor frequency ([Motor Frequency] (RFR))
  lcr = 2,    ///< Motor current ([Motor Current] (LCR))
  spd = 3,    ///< Motor speed ([Motor Speed] (SPD))
  uop = 4,    ///< Motor voltage ([Motor Voltage] (UOP))
  oprw = 5,   ///< Motor mechanical power ([Motor Mech. Power] (OPRW))
  iprw = 6,   ///< Input Electrical power ([Input Elec. Power] (IPRW))
  eprw = 7,   ///< Output Electrical power ([Output Elec. Power] (EPRW))
  otr = 8,    ///< Motor torque ([Motor Torque] (OTR))
  uln = 9,    ///< Mains Voltage ([Mains Voltage] (ULN))
  vbus = 10,  ///< DC BUS Voltage ([DC BUS Voltage] (VBUS))
  rpf = 11,   ///< PID feedback ([PID feedback] (RPF))
  th2v = 12,  ///< AI2 thermal value ([AI2 Th Value] (TH2V))
  th3v = 13,  ///< AI3 thermal value ([AI3 Th Value] (TH3V))
  th4v = 14,  ///< AI4 thermal value ([AI4 Th Value] (TH4V))
  th5v = 15,  ///< AI5 thermal value ([AI5 Th Value] (TH5V))
  thd = 16,   ///< Drive Thermal State ([Drive Thermal State] (THD))
  thr = 17,   ///< Motor Thermal State ([Motor Thermal State] (THR))
  fs1v = 18,  ///< Installation flow ([Installation Flow] (FS1V))
  fs2v = 19,  ///< Pump flow ([Pump Flow] (FS2V))
  ps1v = 20,  ///< Inlet pressure ([Inlet Pressure] (PS1V))
  ps2v = 21,  ///< Outlet pressure ([Outlet Pressure] (PS2V))
  eci = 22,   ///< Energy consumption indicator ([Energy Consum. Ind.] (ECI))
  efy = 23,   ///< Pump efficiency ([Pump efficiency] (EFY))
  epi = 24,   ///< Energy Performance Indicator ([Energy Perf. Ind.] (EPI))
  iln = 25,   ///< Mains current ([Mains Current] (ILN))
  iqrw = 26,  ///< Input reactive power ([Input Reactive Power] (IQRW))
  pwf = 27,   ///< Input power factor ([Input Power Factor] (PWF))
  th1v = 28,  ///< AI1 thermal value ([AI1 Th Value] (TH1V))
  thb = 29,   ///< DBR thermal state ([DBR Thermal State] (THB))
};
[[nodiscard]] constexpr auto enum_desc(ldd_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ldd_e::no:
      return "[Distrib. Log. DISABLE] (NO), Distribution logging disable";
    case ldd_e::rfr:
      return "[Motor Frequency] (RFR), Motor frequency";
    case ldd_e::lcr:
      return "[Motor Current] (LCR), Motor current";
    case ldd_e::spd:
      return "[Motor Speed] (SPD), Motor speed";
    case ldd_e::uop:
      return "[Motor Voltage] (UOP), Motor voltage";
    case ldd_e::oprw:
      return "[Motor Mech. Power] (OPRW), Motor mechanical power";
    case ldd_e::iprw:
      return "[Input Elec. Power] (IPRW), Input Electrical power";
    case ldd_e::eprw:
      return "[Output Elec. Power] (EPRW), Output Electrical power";
    case ldd_e::otr:
      return "[Motor Torque] (OTR), Motor torque";
    case ldd_e::uln:
      return "[Mains Voltage] (ULN), Mains Voltage";
    case ldd_e::vbus:
      return "[DC BUS Voltage] (VBUS), DC BUS Voltage";
    case ldd_e::rpf:
      return "[PID feedback] (RPF), PID feedback";
    case ldd_e::th2v:
      return "[AI2 Th Value] (TH2V), AI2 thermal value";
    case ldd_e::th3v:
      return "[AI3 Th Value] (TH3V), AI3 thermal value";
    case ldd_e::th4v:
      return "[AI4 Th Value] (TH4V), AI4 thermal value";
    case ldd_e::th5v:
      return "[AI5 Th Value] (TH5V), AI5 thermal value";
    case ldd_e::thd:
      return "[Drive Thermal State] (THD), Drive Thermal State";
    case ldd_e::thr:
      return "[Motor Thermal State] (THR), Motor Thermal State";
    case ldd_e::fs1v:
      return "[Installation Flow] (FS1V), Installation flow";
    case ldd_e::fs2v:
      return "[Pump Flow] (FS2V), Pump flow";
    case ldd_e::ps1v:
      return "[Inlet Pressure] (PS1V), Inlet pressure";
    case ldd_e::ps2v:
      return "[Outlet Pressure] (PS2V), Outlet pressure";
    case ldd_e::eci:
      return "[Energy Consum. Ind.] (ECI), Energy consumption indicator";
    case ldd_e::efy:
      return "[Pump efficiency] (EFY), Pump efficiency";
    case ldd_e::epi:
      return "[Energy Perf. Ind.] (EPI), Energy Performance Indicator";
    case ldd_e::iln:
      return "[Mains Current] (ILN), Mains current";
    case ldd_e::iqrw:
      return "[Input Reactive Power] (IQRW), Input reactive power";
    case ldd_e::pwf:
      return "[Input Power Factor] (PWF), Input power factor";
    case ldd_e::th1v:
      return "[AI1 Th Value] (TH1V), AI1 thermal value";
    case ldd_e::thb:
      return "[DBR Thermal State] (THB), DBR thermal state";
  }
  return "unknown";
}
constexpr auto format_as(ldd_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lden_e enum decleration
enum struct lden_e : std::uint16_t {
  stop = 0,    ///< Stop ([Stop] (STOP))
  start = 1,   ///< Start ([Start] (START))
  always = 2,  ///< Always ([Always] (ALWAYS))
  reset = 3,   ///< Reset ([Reset] (RESET))
  clear = 4,   ///< Clear ([Clear] (CLEAR))
  error = 5,   ///< Error ([Error] (ERROR))
};
[[nodiscard]] constexpr auto enum_desc(lden_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lden_e::stop:
      return "[Stop] (STOP), Stop";
    case lden_e::start:
      return "[Start] (START), Start";
    case lden_e::always:
      return "[Always] (ALWAYS), Always";
    case lden_e::reset:
      return "[Reset] (RESET), Reset";
    case lden_e::clear:
      return "[Clear] (CLEAR), Clear";
    case lden_e::error:
      return "[Error] (ERROR), Error";
  }
  return "unknown";
}
constexpr auto format_as(lden_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ldst_e enum decleration
enum struct ldst_e : std::uint16_t {
  milliseconds_200 = 2,  ///< 200 ms ([200 ms] (200MS))
  seconds_1 = 10,        ///< 1 second ([1 second] (1S))
  seconds_2 = 20,        ///< 2 seconds ([2 seconds] (2S))
  seconds_5 = 50,        ///< 5 seconds ([5 seconds] (5S))
};
[[nodiscard]] constexpr auto enum_desc(ldst_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ldst_e::milliseconds_200:
      return "[200 ms] (200MS), 200 ms";
    case ldst_e::seconds_1:
      return "[1 second] (1S), 1 second";
    case ldst_e::seconds_2:
      return "[2 seconds] (2S), 2 seconds";
    case ldst_e::seconds_5:
      return "[5 seconds] (5S), 5 seconds";
  }
  return "unknown";
}
constexpr auto format_as(ldst_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of lft_e enum decleration
enum struct lft_e : std::uint16_t {
  no_fault = 0,  ///< No error detected ([No Error] (NOF))
  eef1 = 2,      ///< EEPROM control ([EEPROM Control] (EEF1))
  cff = 3,       ///< Incorrect configuration ([Incorrect Configuration] (CFF))
  cfi = 4,       ///< Invalid configuration ([Invalid Configuration] (CFI))
  slf1 = 5,      ///< Modbus communication interruption ([Modbus Com Interruption] (SLF1))
  ilf = 6,       ///< Internal communication interruption with option module  ([Internal Link Error] (ILF))
  cnf = 7,       ///< Fieldbus communication interruption ([Fieldbus Com Interrupt] (CNF))
  epf1 = 8,      ///< External detected error ([External Error] (EPF1))
  ocf = 9,       ///< Overcurrent ([Overcurrent] (OCF))
  crf1 = 10,     ///< Precharge capacitor ([Precharge Capacitor] (CRF1))
  lff2 = 13,     ///< AI2 4-20mA loss ([AI2 4-20mA loss] (LFF2))
  ihf = 15,      ///< Input Overheating ([Input Overheating] (IHF))
  ohf = 16,      ///< Drive overheating ([Drive Overheating] (OHF))
  olf = 17,      ///< Motor overload ([Motor Overload] (OLF))
  obf = 18,      ///< DC bus overvoltage ([DC Bus Overvoltage] (OBF))
  osf = 19,      ///< Supply mains overvoltage ([Supply Mains Overvoltage] (OSF))
  opf1 = 20,     ///< Single output phase loss ([Single output phase loss] (OPF1))
  phf = 21,      ///< Input phase loss ([Input phase loss] (PHF))
  usf = 22,      ///< Supply mains undervoltage ([Supply Mains UnderV] (USF))
  scf1 = 23,     ///< Motor short circuit ([Motor short circuit] (SCF1))
  sof = 24,      ///< Motor overspeed ([Motor Overspeed] (SOF))
  tnf = 25,      ///< Autotuning detected error ([Autotuning Error] (TNF))
  inf1 = 26,     ///< Internal error 1 (Rating) ([Internal Error 1] (INF1))
  inf2 = 27,     ///< Internal error 2 (Soft) ([Internal Error 2] (INF2))
  inf3 = 28,     ///< Internal error 3 (Intern Comm) ([Internal Error 3] (INF3))
  inf4 = 29,     ///< Internal error 4 (Manufacturing) ([Internal Error 4] (INF4))
  eef2 = 30,     ///< EEPROM power ([EEPROM Power] (EEF2))
  scf3 = 32,     ///< Ground short circuit ([Ground Short Circuit] (SCF3))
  opf2 = 33,     ///< Output phase loss ([Output Phase Loss] (OPF2))
  cof = 34,      ///< CANopen communication interruption ([CANopen Com Interrupt] (COF))
  inf7 = 37,     ///< Internal error 7 (Init) ([Internal Error 7] (INF7))
  epf2 = 38,     ///< External error detected by Fieldbus ([Fieldbus Error] (EPF2))
  inf8 = 40,     ///< Internal error 8 (Switching Supply) ([Internal Error 8] (INF8))
  brf = 41,      ///< Brake feedback ([Brake feedback] (brF))
  slf2 = 42,     ///< PC communication interruption ([PC Com Interruption] (SLF2))
  ssf = 44,      ///< Torque/current lim ([Torque/current lim] (SSF))
  slf3 = 45,     ///< HMI communication interruption ([HMI Com Interruption] (SLF3))
  ptfl = 49,     ///< PTC probe ([PTC probe] (PtFL))
  inf9 = 51,     ///< Internal error 9 (Measure) ([Internal Error 9] (INF9))
  infa = 52,     ///< Internal error 10 (Mains) ([Internal Error 10] (INFA))
  infb = 53,     ///< Internal error 11 (Temperature) ([Internal Error 11] (INFB))
  tjf = 54,      ///< IGBT overheating ([IGBT Overheating] (TJF))
  scf4 = 55,     ///< IGBT short circuit ([IGBT Short Circuit] (SCF4))
  scf5 = 56,     ///< Motor short circuit ([Motor Short Circuit] (SCF5))
  fcf1 = 58,     ///< Output contactor closed error ([Out Contact Closed Error] (FCF1))
  fcf2 = 59,     ///< Output contactor opened error ([Out Contact Opened Error] (FCF2))
  infc = 60,     ///< Internal error 12 (Internal current supply) ([Internal Error 12] (INFC))
  lcf = 64,      ///< input contactor ([Input Contactor] (LCF))
  inf6 = 68,     ///< Internal error 6 (Option) ([Internal Error 6] (INF6))
  infe = 69,     ///< Internal error 14 (CPU) ([Internal Error 14] (INFE))
  lff3 = 71,     ///< AI3 4-20mA loss ([AI3 4-20mA loss] (LFF3))
  lff4 = 72,     ///< AI4 4-20mA loss ([AI4 4-20mA loss] (LFF4))
  hcf = 73,      ///< Boards compatibility ([Boards Compatibility] (HCF))
  cfi2 = 77,     ///< Configuration transfer error ([Conf Transfer Error] (CFI2))
  lff5 = 79,     ///< AI5 4-20 mA loss ([AI5 4-20 mA loss] (LFF5))
  ffdf = 80,     ///< Fan feedback error ([Fan Feedback Error] (FFDF))
  mof = 81,      ///< Module overheat error ([Module Overheat] (MOF))
  csf = 99,      ///< Channel switching detected error ([Channel Switch Error] (CSF))
  ulf = 100,     ///< Process Underload ([Process Underload] (ULF))
  olc = 101,     ///< Process overload ([Process Overload] (OLC))
  asf = 105,     ///< Angle error ([Angle error] (ASF))
  lff1 = 106,    ///< AI1 4-20 mA loss ([AI1 4-20 mA loss] (LFF1))
  saff = 107,    ///< Safety function detected error ([Safety Function Error] (SAFF))
  th2f = 110,    ///< AI2 thermal level error ([AI2 Th Level Error] (TH2F))
  t2cf = 111,    ///< Thermal sensor error on AI2 ([AI2 Thermal Sensor Error] (T2CF))
  th3f = 112,    ///< AI3 thermal level error ([AI3 Th Level Error] (TH3F))
  t3cf = 113,    ///< Thermal sensor error on AI3 ([AI3 Thermal Sensor Error] (T3CF))
  pcpf = 114,    ///< Pump cycle start error  ([PumpCycle start Error ] (PCPF))
  oplf = 115,    ///< Outlet pressure low ([Out Pressure Low] (OPLF))
  hfpf = 116,    ///< High flow error ([High Flow Error] (HFPF))
  ippf = 117,    ///< Inlet pressure detected error ([Inlet Pressure Error] (IPPF))
  plff = 119,    ///< Pump low flow detected error ([Pump Low Flow Error] (PLFF))
  th4f = 120,    ///< AI4 thermal level error ([AI4 Th Level Error] (TH4F))
  t4cf = 121,    ///< Thermal sensor error on AI4 ([AI4 Thermal Sensor Error] (T4CF))
  th5f = 122,    ///< AI5 thermal level error ([AI5 Th Level Error] (TH5F))
  t5cf = 123,    ///< Thermal sensor error on AI5 ([AI5 Thermal Sensor Error] (T5CF))
  jamf = 124,    ///< Anti Jam detected error ([Anti Jam Error] (JAMF))
  ophf = 125,    ///< Outlet pressure high ([Out Pressure High] (OPHF))
  dryf = 126,    ///< Dry run detected error ([Dry Run Error] (DRYF))
  pfmf = 127,    ///< PID feedback detected error ([PID Feedback Error] (PFMF))
  pglf = 128,    ///< Program loading detected error ([Program Loading Error] (PGLF))
  pgrf = 129,    ///< Program running detected error ([Program Running Error] (PGRF))
  mplf = 130,    ///< Lead pump not available ([Lead Pump Error] (MPLF))
  lclf = 131,    ///< Low level error ([Low Level Error] (LCLF))
  lchf = 132,    ///< High level error ([High Level Error] (LCHF))
  infg = 142,    ///< Internal error 16 (IO module - relay) ([Internal Error 16] (INFG))
  infh = 143,    ///< Internal error 17 (IO module - Standard) ([Internal Error 17] (INFH))
  inf0 = 144,    ///< Internal error 0 (IPC) ([Internal Error 0] (INF0))
  infd = 146,    ///< Internal error 13 (Diff current) ([Internal Error 13] (INFD))
  stf = 148,     ///< Motor stall detected error ([Motor Stall Error] (STF))
  infl = 149,    ///< Internal error 21 (RTC) ([Internal Error 21] (INFL))
  ethf = 150,    ///< Embedded Ethernet communication interruption ([Embd Eth Com Interrupt] (ETHF))
  inff = 151,    ///< Internal error 15 (Flash) ([Internal Error 15] (INFF))
  fwer = 152,    ///< Firmware Update error ([Firmware Update Error] (FWER))
  infm = 153,    ///< Internal error 22 (Embedded Ethernet) ([Internal Error 22] (INFM))
  infp = 154,    ///< Internal error 25 (Incompatibility CB & SW) ([Internal Error 25] (INFP))
  infk = 155,    ///< Internal error 20 (option interface PCBA) ([Internal Error 20] (INFK))
  infr = 157,    ///< Internal error 27 (Diagnostics CPLD) ([Internal Error 27] (INFR))
  infn = 158,    ///< Internal error 23 (Module link) ([Internal Error 23] (INFN))
  scf6 = 159,    ///< AFE ShortCircuit error ([AFE ShortCircuit error] (SCF6))
  obf2 = 160,    ///< AFE Bus unbalancing  ([AFE Bus unbalancing ] (OBF2))
  infs = 161,    ///< Internal error 28 (AFE) ([Internal Error 28] (INFS))
  ifa = 162,     ///< Monitoring circuit A error ([MonitorCircuit A Error] (IFA))
  ifb = 163,     ///< Monitoring circuit B error ([MonitorCircuit B Error] (IFB))
  ifc = 164,     ///< Monitoring circuit C error ([MonitorCircuit C Error] (IFC))
  ifd = 165,     ///< Monitoring circuit D error ([MonitorCircuit D Error] (IFD))
  cfa = 166,     ///< Cabinet circuit A error ([CabinetCircuit A Error] (CFA))
  cfb = 167,     ///< Cabinet circuit B error ([CabinetCircuit B Error] (CFB))
  cfc = 168,     ///< Cabinet circuit C error ([CabinetCircuit C Error] (CFC))
  tfa = 169,     ///< Motor winding A error ([MotorWinding A Error] (TFA))
  tfb = 170,     ///< Motor winding B error ([MotorWinding B Error] (TFB))
  tfc = 171,     ///< Motor bearing A error ([MotorBearing A Error] (TFC))
  tfd = 172,     ///< Motor bearing B error ([MotorBearing B Error] (TFD))
  chf = 173,     ///< Cabinet overheat  error ([Cabinet Overheat  Error] (CHF))
  urf = 174,     ///< AFE Mains undervoltage  ([AFE Mains Undervoltage ] (URF))
  infv = 175,    ///< Internal error 31 (Missing brick) ([Internal Error 31] (INFV))
  inft = 176,    ///< Internal error 29 (Inverter) ([Internal Error 29] (INFT))
  infu = 177,    ///< Internal error 30 (Rectifier) ([Internal Error 30] (INFU))
  tjf2 = 179,    ///< AFE IGBT over-heat error ([AFE IGBT over-heat error] (TJF2))
  crf3 = 180,    ///< AFE contactor feedback error ([AFE contactor feedback error] (CRF3))
  cfi3 = 181,    ///< Pre-settings transfer error ([Pre-settings Transfer Error] (CFI3))
  cbf = 182,     ///< Circuit breaker error ([Circuit Breaker Error] (CBF))
  mdlf = 186,    ///< MultiDrive Link error ([MultiDrive Link Error] (MDLF))
  mpdf = 190,    ///< Multipump device error ([M/P Device Error] (MPDF))
  acf1 = 191,    ///< AFE modulation rate error ([AFE Modulation Rate Error] (ACF1))
  acf2 = 192,    ///< AFE current control error ([AFE Current Control Error] (ACF2))
  mff = 193,     ///< Mains frequency out of range ([Mains Freq Out Of Range] (MFF))
  fdr1 = 200,    ///< FDR Eth embedded error ([FDR 1 Error] (FDR1))
  fdr2 = 201,    ///< FDR Eth module error ([FDR 2 Error] (FDR2))
  p24c = 203,    ///< Cabinet I/O 24V missing error ([Cab I/O 24V Error] (P24C))
  dcre = 206,    ///< DC Bus ripple error ([DC Bus Ripple Error] (DCRE))
  idlf = 208,    ///< Idle mode exit error ([Egy Saving Exit Error] (IDLF))
  spfc = 211,    ///< Security files corrupt ([Security Files Corrupt] (SPFC))
};
[[nodiscard]] constexpr auto enum_desc(lft_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case lft_e::no_fault:
      return "[No Error] (NOF), No error detected";
    case lft_e::eef1:
      return "[EEPROM Control] (EEF1), EEPROM control";
    case lft_e::cff:
      return "[Incorrect Configuration] (CFF), Incorrect configuration";
    case lft_e::cfi:
      return "[Invalid Configuration] (CFI), Invalid configuration";
    case lft_e::slf1:
      return "[Modbus Com Interruption] (SLF1), Modbus communication interruption";
    case lft_e::ilf:
      return "[Internal Link Error] (ILF), Internal communication interruption with option module ";
    case lft_e::cnf:
      return "[Fieldbus Com Interrupt] (CNF), Fieldbus communication interruption";
    case lft_e::epf1:
      return "[External Error] (EPF1), External detected error";
    case lft_e::ocf:
      return "[Overcurrent] (OCF), Overcurrent";
    case lft_e::crf1:
      return "[Precharge Capacitor] (CRF1), Precharge capacitor";
    case lft_e::lff2:
      return "[AI2 4-20mA loss] (LFF2), AI2 4-20mA loss";
    case lft_e::ihf:
      return "[Input Overheating] (IHF), Input Overheating";
    case lft_e::ohf:
      return "[Drive Overheating] (OHF), Drive overheating";
    case lft_e::olf:
      return "[Motor Overload] (OLF), Motor overload";
    case lft_e::obf:
      return "[DC Bus Overvoltage] (OBF), DC bus overvoltage";
    case lft_e::osf:
      return "[Supply Mains Overvoltage] (OSF), Supply mains overvoltage";
    case lft_e::opf1:
      return "[Single output phase loss] (OPF1), Single output phase loss";
    case lft_e::phf:
      return "[Input phase loss] (PHF), Input phase loss";
    case lft_e::usf:
      return "[Supply Mains UnderV] (USF), Supply mains undervoltage";
    case lft_e::scf1:
      return "[Motor short circuit] (SCF1), Motor short circuit";
    case lft_e::sof:
      return "[Motor Overspeed] (SOF), Motor overspeed";
    case lft_e::tnf:
      return "[Autotuning Error] (TNF), Autotuning detected error";
    case lft_e::inf1:
      return "[Internal Error 1] (INF1), Internal error 1 (Rating)";
    case lft_e::inf2:
      return "[Internal Error 2] (INF2), Internal error 2 (Soft)";
    case lft_e::inf3:
      return "[Internal Error 3] (INF3), Internal error 3 (Intern Comm)";
    case lft_e::inf4:
      return "[Internal Error 4] (INF4), Internal error 4 (Manufacturing)";
    case lft_e::eef2:
      return "[EEPROM Power] (EEF2), EEPROM power";
    case lft_e::scf3:
      return "[Ground Short Circuit] (SCF3), Ground short circuit";
    case lft_e::opf2:
      return "[Output Phase Loss] (OPF2), Output phase loss";
    case lft_e::cof:
      return "[CANopen Com Interrupt] (COF), CANopen communication interruption";
    case lft_e::inf7:
      return "[Internal Error 7] (INF7), Internal error 7 (Init)";
    case lft_e::epf2:
      return "[Fieldbus Error] (EPF2), External error detected by Fieldbus";
    case lft_e::inf8:
      return "[Internal Error 8] (INF8), Internal error 8 (Switching Supply)";
    case lft_e::brf:
      return "[Brake feedback] (brF), Brake feedback";
    case lft_e::slf2:
      return "[PC Com Interruption] (SLF2), PC communication interruption";
    case lft_e::ssf:
      return "[Torque/current lim] (SSF), Torque/current lim";
    case lft_e::slf3:
      return "[HMI Com Interruption] (SLF3), HMI communication interruption";
    case lft_e::ptfl:
      return "[PTC probe] (PtFL), PTC probe";
    case lft_e::inf9:
      return "[Internal Error 9] (INF9), Internal error 9 (Measure)";
    case lft_e::infa:
      return "[Internal Error 10] (INFA), Internal error 10 (Mains)";
    case lft_e::infb:
      return "[Internal Error 11] (INFB), Internal error 11 (Temperature)";
    case lft_e::tjf:
      return "[IGBT Overheating] (TJF), IGBT overheating";
    case lft_e::scf4:
      return "[IGBT Short Circuit] (SCF4), IGBT short circuit";
    case lft_e::scf5:
      return "[Motor Short Circuit] (SCF5), Motor short circuit";
    case lft_e::fcf1:
      return "[Out Contact Closed Error] (FCF1), Output contactor closed error";
    case lft_e::fcf2:
      return "[Out Contact Opened Error] (FCF2), Output contactor opened error";
    case lft_e::infc:
      return "[Internal Error 12] (INFC), Internal error 12 (Internal current supply)";
    case lft_e::lcf:
      return "[Input Contactor] (LCF), input contactor";
    case lft_e::inf6:
      return "[Internal Error 6] (INF6), Internal error 6 (Option)";
    case lft_e::infe:
      return "[Internal Error 14] (INFE), Internal error 14 (CPU)";
    case lft_e::lff3:
      return "[AI3 4-20mA loss] (LFF3), AI3 4-20mA loss";
    case lft_e::lff4:
      return "[AI4 4-20mA loss] (LFF4), AI4 4-20mA loss";
    case lft_e::hcf:
      return "[Boards Compatibility] (HCF), Boards compatibility";
    case lft_e::cfi2:
      return "[Conf Transfer Error] (CFI2), Configuration transfer error";
    case lft_e::lff5:
      return "[AI5 4-20 mA loss] (LFF5), AI5 4-20 mA loss";
    case lft_e::ffdf:
      return "[Fan Feedback Error] (FFDF), Fan feedback error";
    case lft_e::mof:
      return "[Module Overheat] (MOF), Module overheat error";
    case lft_e::csf:
      return "[Channel Switch Error] (CSF), Channel switching detected error";
    case lft_e::ulf:
      return "[Process Underload] (ULF), Process Underload";
    case lft_e::olc:
      return "[Process Overload] (OLC), Process overload";
    case lft_e::asf:
      return "[Angle error] (ASF), Angle error";
    case lft_e::lff1:
      return "[AI1 4-20 mA loss] (LFF1), AI1 4-20 mA loss";
    case lft_e::saff:
      return "[Safety Function Error] (SAFF), Safety function detected error";
    case lft_e::th2f:
      return "[AI2 Th Level Error] (TH2F), AI2 thermal level error";
    case lft_e::t2cf:
      return "[AI2 Thermal Sensor Error] (T2CF), Thermal sensor error on AI2";
    case lft_e::th3f:
      return "[AI3 Th Level Error] (TH3F), AI3 thermal level error";
    case lft_e::t3cf:
      return "[AI3 Thermal Sensor Error] (T3CF), Thermal sensor error on AI3";
    case lft_e::pcpf:
      return "[PumpCycle start Error ] (PCPF), Pump cycle start error ";
    case lft_e::oplf:
      return "[Out Pressure Low] (OPLF), Outlet pressure low";
    case lft_e::hfpf:
      return "[High Flow Error] (HFPF), High flow error";
    case lft_e::ippf:
      return "[Inlet Pressure Error] (IPPF), Inlet pressure detected error";
    case lft_e::plff:
      return "[Pump Low Flow Error] (PLFF), Pump low flow detected error";
    case lft_e::th4f:
      return "[AI4 Th Level Error] (TH4F), AI4 thermal level error";
    case lft_e::t4cf:
      return "[AI4 Thermal Sensor Error] (T4CF), Thermal sensor error on AI4";
    case lft_e::th5f:
      return "[AI5 Th Level Error] (TH5F), AI5 thermal level error";
    case lft_e::t5cf:
      return "[AI5 Thermal Sensor Error] (T5CF), Thermal sensor error on AI5";
    case lft_e::jamf:
      return "[Anti Jam Error] (JAMF), Anti Jam detected error";
    case lft_e::ophf:
      return "[Out Pressure High] (OPHF), Outlet pressure high";
    case lft_e::dryf:
      return "[Dry Run Error] (DRYF), Dry run detected error";
    case lft_e::pfmf:
      return "[PID Feedback Error] (PFMF), PID feedback detected error";
    case lft_e::pglf:
      return "[Program Loading Error] (PGLF), Program loading detected error";
    case lft_e::pgrf:
      return "[Program Running Error] (PGRF), Program running detected error";
    case lft_e::mplf:
      return "[Lead Pump Error] (MPLF), Lead pump not available";
    case lft_e::lclf:
      return "[Low Level Error] (LCLF), Low level error";
    case lft_e::lchf:
      return "[High Level Error] (LCHF), High level error";
    case lft_e::infg:
      return "[Internal Error 16] (INFG), Internal error 16 (IO module - relay)";
    case lft_e::infh:
      return "[Internal Error 17] (INFH), Internal error 17 (IO module - Standard)";
    case lft_e::inf0:
      return "[Internal Error 0] (INF0), Internal error 0 (IPC)";
    case lft_e::infd:
      return "[Internal Error 13] (INFD), Internal error 13 (Diff current)";
    case lft_e::stf:
      return "[Motor Stall Error] (STF), Motor stall detected error";
    case lft_e::infl:
      return "[Internal Error 21] (INFL), Internal error 21 (RTC)";
    case lft_e::ethf:
      return "[Embd Eth Com Interrupt] (ETHF), Embedded Ethernet communication interruption";
    case lft_e::inff:
      return "[Internal Error 15] (INFF), Internal error 15 (Flash)";
    case lft_e::fwer:
      return "[Firmware Update Error] (FWER), Firmware Update error";
    case lft_e::infm:
      return "[Internal Error 22] (INFM), Internal error 22 (Embedded Ethernet)";
    case lft_e::infp:
      return "[Internal Error 25] (INFP), Internal error 25 (Incompatibility CB & SW)";
    case lft_e::infk:
      return "[Internal Error 20] (INFK), Internal error 20 (option interface PCBA)";
    case lft_e::infr:
      return "[Internal Error 27] (INFR), Internal error 27 (Diagnostics CPLD)";
    case lft_e::infn:
      return "[Internal Error 23] (INFN), Internal error 23 (Module link)";
    case lft_e::scf6:
      return "[AFE ShortCircuit error] (SCF6), AFE ShortCircuit error";
    case lft_e::obf2:
      return "[AFE Bus unbalancing ] (OBF2), AFE Bus unbalancing ";
    case lft_e::infs:
      return "[Internal Error 28] (INFS), Internal error 28 (AFE)";
    case lft_e::ifa:
      return "[MonitorCircuit A Error] (IFA), Monitoring circuit A error";
    case lft_e::ifb:
      return "[MonitorCircuit B Error] (IFB), Monitoring circuit B error";
    case lft_e::ifc:
      return "[MonitorCircuit C Error] (IFC), Monitoring circuit C error";
    case lft_e::ifd:
      return "[MonitorCircuit D Error] (IFD), Monitoring circuit D error";
    case lft_e::cfa:
      return "[CabinetCircuit A Error] (CFA), Cabinet circuit A error";
    case lft_e::cfb:
      return "[CabinetCircuit B Error] (CFB), Cabinet circuit B error";
    case lft_e::cfc:
      return "[CabinetCircuit C Error] (CFC), Cabinet circuit C error";
    case lft_e::tfa:
      return "[MotorWinding A Error] (TFA), Motor winding A error";
    case lft_e::tfb:
      return "[MotorWinding B Error] (TFB), Motor winding B error";
    case lft_e::tfc:
      return "[MotorBearing A Error] (TFC), Motor bearing A error";
    case lft_e::tfd:
      return "[MotorBearing B Error] (TFD), Motor bearing B error";
    case lft_e::chf:
      return "[Cabinet Overheat  Error] (CHF), Cabinet overheat  error";
    case lft_e::urf:
      return "[AFE Mains Undervoltage ] (URF), AFE Mains undervoltage ";
    case lft_e::infv:
      return "[Internal Error 31] (INFV), Internal error 31 (Missing brick)";
    case lft_e::inft:
      return "[Internal Error 29] (INFT), Internal error 29 (Inverter)";
    case lft_e::infu:
      return "[Internal Error 30] (INFU), Internal error 30 (Rectifier)";
    case lft_e::tjf2:
      return "[AFE IGBT over-heat error] (TJF2), AFE IGBT over-heat error";
    case lft_e::crf3:
      return "[AFE contactor feedback error] (CRF3), AFE contactor feedback error";
    case lft_e::cfi3:
      return "[Pre-settings Transfer Error] (CFI3), Pre-settings transfer error";
    case lft_e::cbf:
      return "[Circuit Breaker Error] (CBF), Circuit breaker error";
    case lft_e::mdlf:
      return "[MultiDrive Link Error] (MDLF), MultiDrive Link error";
    case lft_e::mpdf:
      return "[M/P Device Error] (MPDF), Multipump device error";
    case lft_e::acf1:
      return "[AFE Modulation Rate Error] (ACF1), AFE modulation rate error";
    case lft_e::acf2:
      return "[AFE Current Control Error] (ACF2), AFE current control error";
    case lft_e::mff:
      return "[Mains Freq Out Of Range] (MFF), Mains frequency out of range";
    case lft_e::fdr1:
      return "[FDR 1 Error] (FDR1), FDR Eth embedded error";
    case lft_e::fdr2:
      return "[FDR 2 Error] (FDR2), FDR Eth module error";
    case lft_e::p24c:
      return "[Cab I/O 24V Error] (P24C), Cabinet I/O 24V missing error";
    case lft_e::dcre:
      return "[DC Bus Ripple Error] (DCRE), DC Bus ripple error";
    case lft_e::idlf:
      return "[Egy Saving Exit Error] (IDLF), Idle mode exit error";
    case lft_e::spfc:
      return "[Security Files Corrupt] (SPFC), Security files corrupt";
  }
  return "unknown";
}
constexpr auto format_as(lft_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mdt_e enum decleration
enum struct mdt_e : std::uint16_t {
  dec = 0,    ///< Digital values ([Digital] (DEC))
  bar = 1,    ///< Bar graph ([Bar graph] (BAR))
  list = 2,   ///< List of values ([List] (LIST))
  vumet = 3,  ///< Vu Meter ([Vu Meter] (VUMET))
};
[[nodiscard]] constexpr auto enum_desc(mdt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mdt_e::dec:
      return "[Digital] (DEC), Digital values";
    case mdt_e::bar:
      return "[Bar graph] (BAR), Bar graph";
    case mdt_e::list:
      return "[List] (LIST), List of values";
    case mdt_e::vumet:
      return "[Vu Meter] (VUMET), Vu Meter";
  }
  return "unknown";
}
constexpr auto format_as(mdt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mpc_e enum decleration
enum struct mpc_e : std::uint16_t {
  npr = 0,  ///< Nominal motor power ([Mot Power] (NPR))
  cos = 1,  ///< Nominal motor cosinus Phi ([Mot Cosinus] (COS))
};
[[nodiscard]] constexpr auto enum_desc(mpc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mpc_e::npr:
      return "[Mot Power] (NPR), Nominal motor power";
    case mpc_e::cos:
      return "[Mot Cosinus] (COS), Nominal motor cosinus Phi";
  }
  return "unknown";
}
constexpr auto format_as(mpc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mpdt_e enum decleration
enum struct mpdt_e : std::uint16_t {
  slave = 0,  ///< Slave ([Slave] (SLAVE))
  mast = 1,   ///< Master ([Master] (MAST))
  mast1 = 2,  ///< Master only ([Master Only] (MAST1))
  mast2 = 3,  ///< Master or slave ([Master or Slave] (MAST2))
};
[[nodiscard]] constexpr auto enum_desc(mpdt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mpdt_e::slave:
      return "[Slave] (SLAVE), Slave";
    case mpdt_e::mast:
      return "[Master] (MAST), Master";
    case mpdt_e::mast1:
      return "[Master Only] (MAST1), Master only";
    case mpdt_e::mast2:
      return "[Master or Slave] (MAST2), Master or slave";
  }
  return "unknown";
}
constexpr auto format_as(mpdt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mpla_e enum decleration
enum struct mpla_e : std::uint16_t {
  no = 0,   ///< Deactivated ([No] (NO))
  yes = 1,  ///< Standard alternation ([Standard] (YES))
  red = 2,  ///< Redundancy mode ([Redundancy] (RED))
};
[[nodiscard]] constexpr auto enum_desc(mpla_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mpla_e::no:
      return "[No] (NO), Deactivated";
    case mpla_e::yes:
      return "[Standard] (YES), Standard alternation";
    case mpla_e::red:
      return "[Redundancy] (RED), Redundancy mode";
  }
  return "unknown";
}
constexpr auto format_as(mpla_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mppc_e enum decleration
enum struct mppc_e : std::uint16_t {
  fifo = 0,   ///< First In First Out ([FIFO] (FIFO))
  lifo = 1,   ///< Last In First Out ([LIFO] (LIFO))
  rtime = 2,  ///< Pump runtime ([Runtime] (RTIME))
  rtlf = 3,   ///< Runtime&Last In First Out ([Runtime&LIFO] (RTLF))
};
[[nodiscard]] constexpr auto enum_desc(mppc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mppc_e::fifo:
      return "[FIFO] (FIFO), First In First Out";
    case mppc_e::lifo:
      return "[LIFO] (LIFO), Last In First Out";
    case mppc_e::rtime:
      return "[Runtime] (RTIME), Pump runtime";
    case mppc_e::rtlf:
      return "[Runtime&LIFO] (RTLF), Runtime&Last In First Out";
  }
  return "unknown";
}
constexpr auto format_as(mppc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mps_e enum decleration
enum struct mps_e : std::uint16_t {
  none = 0,   ///< None ([None] (NONE))
  ready = 1,  ///< Ready ([Ready] (READY))
  run = 2,    ///< Running ([Running] (RUN))
  alarm = 3,  ///< warning ([Warning] (ALARM))
  fault = 4,  ///< Error ([Error] (FAULT))
  navl = 5,   ///< Not available ([Not Available] (NAVL))
};
[[nodiscard]] constexpr auto enum_desc(mps_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mps_e::none:
      return "[None] (NONE), None";
    case mps_e::ready:
      return "[Ready] (READY), Ready";
    case mps_e::run:
      return "[Running] (RUN), Running";
    case mps_e::alarm:
      return "[Warning] (ALARM), warning";
    case mps_e::fault:
      return "[Error] (FAULT), Error";
    case mps_e::navl:
      return "[Not Available] (NAVL), Not available";
  }
  return "unknown";
}
constexpr auto format_as(mps_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mpsa_e enum decleration
enum struct mpsa_e : std::uint16_t {
  no = 0,     ///< Mono-Pump ([Mono-Pump] (NO))
  vndol = 1,  ///< Single Drive ([Single Drive] (VNDOL))
  nvsd = 2,   ///< Multiple Drives ([Multi Drives] (NVSD))
  nvsdr = 3,  ///< Multiple Drives with Master redundancy ([Multi Masters] (NVSDR))
};
[[nodiscard]] constexpr auto enum_desc(mpsa_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mpsa_e::no:
      return "[Mono-Pump] (NO), Mono-Pump";
    case mpsa_e::vndol:
      return "[Single Drive] (VNDOL), Single Drive";
    case mpsa_e::nvsd:
      return "[Multi Drives] (NVSD), Multiple Drives";
    case mpsa_e::nvsdr:
      return "[Multi Masters] (NVSDR), Multiple Drives with Master redundancy";
  }
  return "unknown";
}
constexpr auto format_as(mpsa_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of mpst_e enum decleration
enum struct mpst_e : std::uint16_t {
  tmc = 0,   ///< Standard multipump speed control mode ([Standard] (TMC))
  dmc = 1,   ///< Distributed multipump speed control mode ([Distributed] (DMC))
  amc = 2,   ///< Advanced multipump speed control mode ([Advanced] (AMC))
  sync = 3,  ///< Synchronized multipump speed control mode ([Synchronized] (SYNC))
};
[[nodiscard]] constexpr auto enum_desc(mpst_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case mpst_e::tmc:
      return "[Standard] (TMC), Standard multipump speed control mode";
    case mpst_e::dmc:
      return "[Distributed] (DMC), Distributed multipump speed control mode";
    case mpst_e::amc:
      return "[Advanced] (AMC), Advanced multipump speed control mode";
    case mpst_e::sync:
      return "[Synchronized] (SYNC), Synchronized multipump speed control mode";
  }
  return "unknown";
}
constexpr auto format_as(mpst_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ncv_e enum decleration
enum struct ncv_e : std::uint16_t {
  no = 0,     ///< Unknown rating ([Unknown rating] (NO))
  u010 = 1,   ///< 0.10kW - 0.2HP ([0.12kW] (U010))
  u018 = 2,   ///< 0.18 kW / 0.25 Hp ([0.18kW / 0.25Hp] (U018))
  u025 = 3,   ///< 0.25kW - 0.37HP ([0.25kW] (U025))
  u037 = 4,   ///< 0.37 kW / 0.5 Hp ([0.37 kW / 0.5 Hp] (U037))
  u055 = 5,   ///< 0.55 kW / 0.75 Hp ([0.55 kW /0.75 Hp] (U055))
  u075 = 6,   ///< 0.75 kW / 1 Hp ([0.75 kW / 1 Hp] (U075))
  u090 = 7,   ///< 5.5kW - 7.5HP ([5.5kW / 7.5HP] (U090))
  u110 = 8,   ///< 1.1 kW / 1.5 Hp ([1.1 kW / 1.5 Hp] (U110))
  u150 = 9,   ///< 1.5 kW / 2 Hp ([1.5 kW / 2 Hp] (U150))
  u185 = 10,  ///< 1.85kW - 3HP ([1.85kW] (U185))
  u220 = 11,  ///< 2.2 kW / 3 Hp ([2.2 kW / 3 Hp] (U220))
  u300 = 12,  ///< 3 kW / 4HP ([3 kW / 4HP] (U300))
  u370 = 13,  ///< 3.7kW - 5HP ([4kW / 5HP] (U370))
  u400 = 14,  ///< 4kW - 5HP ([4kW / 5HP] (U400))
  u550 = 15,  ///< 5.5 kW / 7.5 Hp ([5.5 kW / 7.5 Hp] (U550))
  u750 = 16,  ///< 7.5 kW / 10 Hp ([7.5 kW / 10 Hp] (U750))
  u900 = 17,  ///< 9kW - 11HP ([9kW] (U900))
  d110 = 18,  ///< 11 kW / 15 HP ([11 kW / 15 Hp] (D110))
  d150 = 19,  ///< 15 kW / 20 HP ([15 kW / 20 Hp] (D150))
  d185 = 20,  ///< 18.5kW - 25HP ([18,5kW / 25HP] (D185))
  d220 = 21,  ///< 22kW - 30HP ([22kW / 30HP] (D220))
  d300 = 22,  ///< 30kW - 40HP ([30kW / 40HP] (D300))
  d370 = 23,  ///< 37kW - 50HP ([37kW / 50HP] (D370))
  d450 = 24,  ///< 45kW - 60HP ([45kW / 60HP] (D450))
  d550 = 25,  ///< 55kW - 75HP ([55kW / 75HP] (D550))
  d750 = 26,  ///< 75kW - 100HP ([75kW / 100HP] (D750))
  d900 = 27,  ///< 90kW - 125HP ([90kW / 125HP] (D900))
  c110 = 28,  ///< 110kW - 150HP ([110 kW / 150HP] (C110))
  c132 = 29,  ///< 132kW - 200HP ([132kW / 200 HP] (C132))
  c160 = 30,  ///< 160kW - 250HP ([160kW / 250HP] (C160))
  c200 = 31,  ///< 200kW - 300HP ([200kW / 300HP] (C200))
  c220 = 32,  ///< 220kW - 350HP ([220kW / 350HP] (C220))
  c250 = 33,  ///< 250kW - 400HP ([250kW / 400HP] (C250))
  c280 = 34,  ///< 280kW - 450HP ([280kW / 450HP] (C280))
  c315 = 35,  ///< 315kW - 500HP ([315kW / 500HP] (C315))
  c355 = 36,  ///< 355kW - 450HP ([355 kW / 450HP] (C355))
  c400 = 37,  ///< 400kW - 600HP ([400kW / 600HP] (C400))
  c450 = 38,  ///< 450kW - 750HP ([450kW / 750HP] (C450))
  c500 = 39,  ///< 500kW - 800HP ([500kW / 800HP] (C500))
  c560 = 40,  ///< 560kW - 850HP ([560kW / 850HP] (C560))
  c630 = 41,  ///< 630kW -900HP ([630kW / 900HP] (C630))
  c710 = 42,  ///< 710kW -950HP ([710kW / 950HP] (C710))
  c800 = 43,  ///< 800kW - 1000HP ([800kW / 1000HP] (C800))
  c900 = 44,  ///< 900kW - 900HP ([900kW / 900HP] (C900))
  m100 = 45,  ///< 1000kW - 1000HP ([1000kW / 1000HP] (M100))
  m110 = 46,  ///< 1100kW - 1100HP ([1100kW / 1100HP] (M110))
  m120 = 47,  ///< 1200kW - 1200HP ([1200kW / 1200HP] (M120))
  m130 = 48,  ///< 1300kW - 1300HP ([1300kW / 1300HP] (M130))
  m140 = 49,  ///< 1400kW - 1400HP ([1400kW / 1400HP] (M140))
  m150 = 50,  ///< 1500kW - 1500HP ([1500kW / 1500HP] (M150))
  m160 = 51,  ///< 1600kW - 1600HP ([1600kW / 1600HP] (M160))
  m170 = 52,  ///< 1700kW - 1700HP ([1700kW / 1700HP] (M170))
  m180 = 53,  ///< 1800kW - 1800HP ([1800kW / 1800HP] (M180))
  m190 = 54,  ///< 1900kW - 1900HP ([1900kW / 1900HP] (M190))
  m200 = 55,  ///< 2000kW - 2000HP ([2000kW / 2000HP] (M200))
  m210 = 56,  ///< 2100kW - 2100HP ([2100kW / 2100HP] (M210))
  m220 = 57,  ///< 2200kW - 2200HP ([2200kW / 2200HP] (M220))
  m230 = 58,  ///< 2300kW - 2300HP ([2300kW / 2300HP] (M230))
  m240 = 59,  ///< 2400kW - 2400HP ([2400kW / 2400HP] (M240))
  m250 = 60,  ///< 2500 kW / 3333 Hp ([2500 kW / 3333 Hp] (M250))
  m260 = 61,  ///< 2600 kW / 3467 Hp ([2600 kW / 3467 Hp] (M260))
};
[[nodiscard]] constexpr auto enum_desc(ncv_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ncv_e::no:
      return "[Unknown rating] (NO), Unknown rating";
    case ncv_e::u010:
      return "[0.12kW] (U010), 0.10kW - 0.2HP";
    case ncv_e::u018:
      return "[0.18kW / 0.25Hp] (U018), 0.18 kW / 0.25 Hp";
    case ncv_e::u025:
      return "[0.25kW] (U025), 0.25kW - 0.37HP";
    case ncv_e::u037:
      return "[0.37 kW / 0.5 Hp] (U037), 0.37 kW / 0.5 Hp";
    case ncv_e::u055:
      return "[0.55 kW /0.75 Hp] (U055), 0.55 kW / 0.75 Hp";
    case ncv_e::u075:
      return "[0.75 kW / 1 Hp] (U075), 0.75 kW / 1 Hp";
    case ncv_e::u090:
      return "[5.5kW / 7.5HP] (U090), 5.5kW - 7.5HP";
    case ncv_e::u110:
      return "[1.1 kW / 1.5 Hp] (U110), 1.1 kW / 1.5 Hp";
    case ncv_e::u150:
      return "[1.5 kW / 2 Hp] (U150), 1.5 kW / 2 Hp";
    case ncv_e::u185:
      return "[1.85kW] (U185), 1.85kW - 3HP";
    case ncv_e::u220:
      return "[2.2 kW / 3 Hp] (U220), 2.2 kW / 3 Hp";
    case ncv_e::u300:
      return "[3 kW / 4HP] (U300), 3 kW / 4HP";
    case ncv_e::u370:
      return "[4kW / 5HP] (U370), 3.7kW - 5HP";
    case ncv_e::u400:
      return "[4kW / 5HP] (U400), 4kW - 5HP";
    case ncv_e::u550:
      return "[5.5 kW / 7.5 Hp] (U550), 5.5 kW / 7.5 Hp";
    case ncv_e::u750:
      return "[7.5 kW / 10 Hp] (U750), 7.5 kW / 10 Hp";
    case ncv_e::u900:
      return "[9kW] (U900), 9kW - 11HP";
    case ncv_e::d110:
      return "[11 kW / 15 Hp] (D110), 11 kW / 15 HP";
    case ncv_e::d150:
      return "[15 kW / 20 Hp] (D150), 15 kW / 20 HP";
    case ncv_e::d185:
      return "[18,5kW / 25HP] (D185), 18.5kW - 25HP";
    case ncv_e::d220:
      return "[22kW / 30HP] (D220), 22kW - 30HP";
    case ncv_e::d300:
      return "[30kW / 40HP] (D300), 30kW - 40HP";
    case ncv_e::d370:
      return "[37kW / 50HP] (D370), 37kW - 50HP";
    case ncv_e::d450:
      return "[45kW / 60HP] (D450), 45kW - 60HP";
    case ncv_e::d550:
      return "[55kW / 75HP] (D550), 55kW - 75HP";
    case ncv_e::d750:
      return "[75kW / 100HP] (D750), 75kW - 100HP";
    case ncv_e::d900:
      return "[90kW / 125HP] (D900), 90kW - 125HP";
    case ncv_e::c110:
      return "[110 kW / 150HP] (C110), 110kW - 150HP";
    case ncv_e::c132:
      return "[132kW / 200 HP] (C132), 132kW - 200HP";
    case ncv_e::c160:
      return "[160kW / 250HP] (C160), 160kW - 250HP";
    case ncv_e::c200:
      return "[200kW / 300HP] (C200), 200kW - 300HP";
    case ncv_e::c220:
      return "[220kW / 350HP] (C220), 220kW - 350HP";
    case ncv_e::c250:
      return "[250kW / 400HP] (C250), 250kW - 400HP";
    case ncv_e::c280:
      return "[280kW / 450HP] (C280), 280kW - 450HP";
    case ncv_e::c315:
      return "[315kW / 500HP] (C315), 315kW - 500HP";
    case ncv_e::c355:
      return "[355 kW / 450HP] (C355), 355kW - 450HP";
    case ncv_e::c400:
      return "[400kW / 600HP] (C400), 400kW - 600HP";
    case ncv_e::c450:
      return "[450kW / 750HP] (C450), 450kW - 750HP";
    case ncv_e::c500:
      return "[500kW / 800HP] (C500), 500kW - 800HP";
    case ncv_e::c560:
      return "[560kW / 850HP] (C560), 560kW - 850HP";
    case ncv_e::c630:
      return "[630kW / 900HP] (C630), 630kW -900HP";
    case ncv_e::c710:
      return "[710kW / 950HP] (C710), 710kW -950HP";
    case ncv_e::c800:
      return "[800kW / 1000HP] (C800), 800kW - 1000HP";
    case ncv_e::c900:
      return "[900kW / 900HP] (C900), 900kW - 900HP";
    case ncv_e::m100:
      return "[1000kW / 1000HP] (M100), 1000kW - 1000HP";
    case ncv_e::m110:
      return "[1100kW / 1100HP] (M110), 1100kW - 1100HP";
    case ncv_e::m120:
      return "[1200kW / 1200HP] (M120), 1200kW - 1200HP";
    case ncv_e::m130:
      return "[1300kW / 1300HP] (M130), 1300kW - 1300HP";
    case ncv_e::m140:
      return "[1400kW / 1400HP] (M140), 1400kW - 1400HP";
    case ncv_e::m150:
      return "[1500kW / 1500HP] (M150), 1500kW - 1500HP";
    case ncv_e::m160:
      return "[1600kW / 1600HP] (M160), 1600kW - 1600HP";
    case ncv_e::m170:
      return "[1700kW / 1700HP] (M170), 1700kW - 1700HP";
    case ncv_e::m180:
      return "[1800kW / 1800HP] (M180), 1800kW - 1800HP";
    case ncv_e::m190:
      return "[1900kW / 1900HP] (M190), 1900kW - 1900HP";
    case ncv_e::m200:
      return "[2000kW / 2000HP] (M200), 2000kW - 2000HP";
    case ncv_e::m210:
      return "[2100kW / 2100HP] (M210), 2100kW - 2100HP";
    case ncv_e::m220:
      return "[2200kW / 2200HP] (M220), 2200kW - 2200HP";
    case ncv_e::m230:
      return "[2300kW / 2300HP] (M230), 2300kW - 2300HP";
    case ncv_e::m240:
      return "[2400kW / 2400HP] (M240), 2400kW - 2400HP";
    case ncv_e::m250:
      return "[2500 kW / 3333 Hp] (M250), 2500 kW / 3333 Hp";
    case ncv_e::m260:
      return "[2600 kW / 3467 Hp] (M260), 2600 kW / 3467 Hp";
  }
  return "unknown";
}
constexpr auto format_as(ncv_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of nmts_e enum decleration
enum struct nmts_e : std::uint16_t {
  boot = 0,  ///< On boot up ([Boot] (BOOT))
  stop = 2,  ///< Stopped ([Stopped] (STOP))
  ope = 1,   ///< Operational ([Operation] (OPE))
  pope = 4,  ///< Pre operation ([Pre-op] (POPE))
};
[[nodiscard]] constexpr auto enum_desc(nmts_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case nmts_e::boot:
      return "[Boot] (BOOT), On boot up";
    case nmts_e::stop:
      return "[Stopped] (STOP), Stopped";
    case nmts_e::ope:
      return "[Operation] (OPE), Operational";
    case nmts_e::pope:
      return "[Pre-op] (POPE), Pre operation";
  }
  return "unknown";
}
constexpr auto format_as(nmts_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of npl_e enum decleration
enum struct npl_e : std::uint16_t {
  pos = 0,  ///< 1 ([1] (POS))
  neg = 1,  ///< 0 ([0] (NEG))
};
[[nodiscard]] constexpr auto enum_desc(npl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case npl_e::pos:
      return "[1] (POS), 1";
    case npl_e::neg:
      return "[0] (NEG), 0";
  }
  return "unknown";
}
constexpr auto format_as(npl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of n_y_e enum decleration
enum struct n_y_e : std::uint16_t {
  no = 0,   ///< No ([No] (NO))
  yes = 1,  ///< Yes ([Yes] (YES))
};
[[nodiscard]] constexpr auto enum_desc(n_y_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case n_y_e::no:
      return "[No] (NO), No";
    case n_y_e::yes:
      return "[Yes] (YES), Yes";
  }
  return "unknown";
}
constexpr auto format_as(n_y_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of opl_e enum decleration
enum struct opl_e : std::uint16_t {
  no = 0,   ///< Function inactive ([Function Inactive] (NO))
  yes = 1,  ///< OPF error Triggered ([OPF Error Triggered] (YES))
  oac = 2,  ///< No error triggered ([No Error Triggered] (OAC))
};
[[nodiscard]] constexpr auto enum_desc(opl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case opl_e::no:
      return "[Function Inactive] (NO), Function inactive";
    case opl_e::yes:
      return "[OPF Error Triggered] (YES), OPF error Triggered";
    case opl_e::oac:
      return "[No Error Triggered] (OAC), No error triggered";
  }
  return "unknown";
}
constexpr auto format_as(opl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of oppm_e enum decleration
enum struct oppm_e : std::uint16_t {
  no = 0,    ///< No ([No] (NO))
  sw = 1,    ///< Switch ([Switch] (SW))
  snsr = 2,  ///< Sensor ([Sensor] (SNSR))
  both = 3,  ///< Both ([Both] (BOTH))
};
[[nodiscard]] constexpr auto enum_desc(oppm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case oppm_e::no:
      return "[No] (NO), No";
    case oppm_e::sw:
      return "[Switch] (SW), Switch";
    case oppm_e::snsr:
      return "[Sensor] (SNSR), Sensor";
    case oppm_e::both:
      return "[Both] (BOTH), Both";
  }
  return "unknown";
}
constexpr auto format_as(oppm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ovma_e enum decleration
enum struct ovma_e : std::uint16_t {
  def = 0,     ///< Default ([Default] (DEFAULT))
  full = 255,  ///< Full ([Full] (FULL))
};
[[nodiscard]] constexpr auto enum_desc(ovma_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ovma_e::def:
      return "[Default] (DEFAULT), Default";
    case ovma_e::full:
      return "[Full] (FULL), Full";
  }
  return "unknown";
}
constexpr auto format_as(ovma_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pcm_e enum decleration
enum struct pcm_e : std::uint16_t {
  no = 0,   ///< Disable pump characteristics ([No] (NO))
  hq = 1,   ///< Enable Head vs flow curve ([HQ] (HQ))
  pq = 2,   ///< Enable power vs flow curve ([PQ] (PQ))
  phq = 3,  ///< Enable Head vs flow and power vs flow curves ([PHQ] (PHQ))
};
[[nodiscard]] constexpr auto enum_desc(pcm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pcm_e::no:
      return "[No] (NO), Disable pump characteristics";
    case pcm_e::hq:
      return "[HQ] (HQ), Enable Head vs flow curve";
    case pcm_e::pq:
      return "[PQ] (PQ), Enable power vs flow curve";
    case pcm_e::phq:
      return "[PHQ] (PHQ), Enable Head vs flow and power vs flow curves";
  }
  return "unknown";
}
constexpr auto format_as(pcm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pcpm_e enum decleration
enum struct pcpm_e : std::uint16_t {
  no = 0,    ///< Pump cycle monitoring disabled ([No] (NO))
  norm = 1,  ///< Pump cycle monitoring mode 1  ([Mode 1] (NORM))
  rtc = 2,   ///< Pump cycle monitoring mode 2  ([Mode 2] (RTC))
};
[[nodiscard]] constexpr auto enum_desc(pcpm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pcpm_e::no:
      return "[No] (NO), Pump cycle monitoring disabled";
    case pcpm_e::norm:
      return "[Mode 1] (NORM), Pump cycle monitoring mode 1 ";
    case pcpm_e::rtc:
      return "[Mode 2] (RTC), Pump cycle monitoring mode 2 ";
  }
  return "unknown";
}
constexpr auto format_as(pcpm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pcs_e enum decleration
enum struct pcs_e : std::uint16_t {
  none = 0,    ///< None ([None] (NONE))
  nact = 1,    ///< Inactive ([Inactive] (NACT))
  active = 2,  ///< Active ([Active] (ACTIVE))
  failed = 3,  ///< Failed ([Failed] (FAILED))
};
[[nodiscard]] constexpr auto enum_desc(pcs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pcs_e::none:
      return "[None] (NONE), None";
    case pcs_e::nact:
      return "[Inactive] (NACT), Inactive";
    case pcs_e::active:
      return "[Active] (ACTIVE), Active";
    case pcs_e::failed:
      return "[Failed] (FAILED), Failed";
  }
  return "unknown";
}
constexpr auto format_as(pcs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pfm_e enum decleration
enum struct pfm_e : std::uint16_t {
  no = 0,   ///< No ([No] (NO))
  fbk = 1,  ///< Pipe-Fill on PID feedback ([Feedback] (FBK))
  ps2 = 2,  ///< Pipe-Fill on Outlet pressure ([Outlet Pressure] (PS2))
};
[[nodiscard]] constexpr auto enum_desc(pfm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pfm_e::no:
      return "[No] (NO), No";
    case pfm_e::fbk:
      return "[Feedback] (FBK), Pipe-Fill on PID feedback";
    case pfm_e::ps2:
      return "[Outlet Pressure] (PS2), Pipe-Fill on Outlet pressure";
  }
  return "unknown";
}
constexpr auto format_as(pfm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of phr_e enum decleration
enum struct phr_e : std::uint16_t {
  abc = 0,  ///< A  - B - C phase rotation ([ABC] (ABC))
  acb = 1,  ///< A  - C - B phase rotation ([ACB] (ACB))
};
[[nodiscard]] constexpr auto enum_desc(phr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case phr_e::abc:
      return "[ABC] (ABC), A  - B - C phase rotation";
    case phr_e::acb:
      return "[ACB] (ACB), A  - C - B phase rotation";
  }
  return "unknown";
}
constexpr auto format_as(phr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pktp_e enum decleration
enum struct pktp_e : std::uint16_t {
  prd = 0,  ///< Product package ([Product] (PRD))
  opt = 1,  ///< Module package ([Module] (OPT))
  spr = 2,  ///< Spare parts package ([Spare parts] (SPR))
  cus = 3,  ///< Customized package ([Customized] (CUS))
  ind = 4,  ///< Indus package ([Indus] (IND))
};
[[nodiscard]] constexpr auto enum_desc(pktp_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pktp_e::prd:
      return "[Product] (PRD), Product package";
    case pktp_e::opt:
      return "[Module] (OPT), Module package";
    case pktp_e::spr:
      return "[Spare parts] (SPR), Spare parts package";
    case pktp_e::cus:
      return "[Customized] (CUS), Customized package";
    case pktp_e::ind:
      return "[Indus] (IND), Indus package";
  }
  return "unknown";
}
constexpr auto format_as(pktp_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of plfm_e enum decleration
enum struct plfm_e : std::uint16_t {
  no = 0,  ///< No ([No] (NO))
  sw = 1,  ///< SW ([Switch] (SW))
  q = 2,   ///< Q ([Flow] (Q))
  qn = 3,  ///< QN ([Flow vs Speed] (QN))
  nf = 5,  ///< NF ([No Flow Power] (NF))
};
[[nodiscard]] constexpr auto enum_desc(plfm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case plfm_e::no:
      return "[No] (NO), No";
    case plfm_e::sw:
      return "[Switch] (SW), SW";
    case plfm_e::q:
      return "[Flow] (Q), Q";
    case plfm_e::qn:
      return "[Flow vs Speed] (QN), QN";
    case plfm_e::nf:
      return "[No Flow Power] (NF), NF";
  }
  return "unknown";
}
constexpr auto format_as(plfm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pmdt_e enum decleration
enum struct pmdt_e : std::uint16_t {
  no = 0,   ///< Parameters list ([Parameters List] (NO))
  hot = 1,  ///< Operating time histogram ([Operating Time] (HOT))
  hns = 2,  ///< Nb of Starts histogram ([Nb of Starts] (HNS))
  eff = 3,  ///< Efficiency trend view ([Efficiency] (EFF))
  cpq = 4,  ///< Power vs flow curve ([Power vs Flow] (CPQ))
  chq = 5,  ///< Head vs flow curve ([Head vs Flow] (CHQ))
  ceq = 6,  ///< Efficiency vs flow curve ([Efficiency vs Flow] (CEQ))
};
[[nodiscard]] constexpr auto enum_desc(pmdt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pmdt_e::no:
      return "[Parameters List] (NO), Parameters list";
    case pmdt_e::hot:
      return "[Operating Time] (HOT), Operating time histogram";
    case pmdt_e::hns:
      return "[Nb of Starts] (HNS), Nb of Starts histogram";
    case pmdt_e::eff:
      return "[Efficiency] (EFF), Efficiency trend view";
    case pmdt_e::cpq:
      return "[Power vs Flow] (CPQ), Power vs flow curve";
    case pmdt_e::chq:
      return "[Head vs Flow] (CHQ), Head vs flow curve";
    case pmdt_e::ceq:
      return "[Efficiency vs Flow] (CEQ), Efficiency vs flow curve";
  }
  return "unknown";
}
constexpr auto format_as(pmdt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pnid_e enum decleration
enum struct pnid_e : std::uint16_t {
  none = 0,  ///< None ([None] (NONE))
  p01 = 1,   ///< Pump 1 ([Pump 1] (P01))
  p02 = 2,   ///< Pump 2 ([Pump 2] (P02))
  p03 = 3,   ///< Pump 3 ([Pump 3] (P03))
  p04 = 4,   ///< Pump 4 ([Pump 4] (P04))
  p05 = 5,   ///< Pump 5 ([Pump 5] (P05))
  p06 = 6,   ///< Pump 6 ([Pump 6] (P06))
};
[[nodiscard]] constexpr auto enum_desc(pnid_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pnid_e::none:
      return "[None] (NONE), None";
    case pnid_e::p01:
      return "[Pump 1] (P01), Pump 1";
    case pnid_e::p02:
      return "[Pump 2] (P02), Pump 2";
    case pnid_e::p03:
      return "[Pump 3] (P03), Pump 3";
    case pnid_e::p04:
      return "[Pump 4] (P04), Pump 4";
    case pnid_e::p05:
      return "[Pump 5] (P05), Pump 5";
    case pnid_e::p06:
      return "[Pump 6] (P06), Pump 6";
  }
  return "unknown";
}
constexpr auto format_as(pnid_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of prfl_e enum decleration
enum struct prfl_e : std::uint16_t {
  uncg = 0,           ///< Not configured ([Not Configured] (UNCG))
  numeric_1 = 1,      ///< 1 ([1] (1))
  numeric_100 = 100,  ///< 100 ([100] (100))
  numeric_101 = 101,  ///< 101 ([101] (101))
  numeric_102 = 102,  ///< 102 ([102] (102))
  numeric_106 = 106,  ///< 106 ([106] (106))
  numeric_107 = 107,  ///< 107 ([107] (107))
};
[[nodiscard]] constexpr auto enum_desc(prfl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case prfl_e::uncg:
      return "[Not Configured] (UNCG), Not configured";
    case prfl_e::numeric_1:
      return "[1] (1), 1";
    case prfl_e::numeric_100:
      return "[100] (100), 100";
    case prfl_e::numeric_101:
      return "[101] (101), 101";
    case prfl_e::numeric_102:
      return "[102] (102), 102";
    case prfl_e::numeric_106:
      return "[106] (106), 106";
    case prfl_e::numeric_107:
      return "[107] (107), 107";
  }
  return "unknown";
}
constexpr auto format_as(prfl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of psa_e enum decleration
enum struct psa_e : uint16_t {
  not_configured = 0,                             ///< Not configured ([Not Configured] (NO))
  analog_input_1 = 1,                             ///< AI1 Analog input ([AI1] (AI1))
  analog_input_2 = 2,                             ///< AI2 Analog input ([AI2] (AI2))
  analog_input_3 = 3,                             ///< AI3 Analog input ([AI3] (AI3))
  analog_input_4 = 4,                             ///< AI4 Analog input ([AI4] (AI4))
  analog_input_5 = 5,                             ///< AI5 Analog input ([AI5] (AI5))
  motor_current = 129,                            ///< Motor current ([Motor Current] (OCR))
  motor_frequency = 130,                          ///< Motor frequency ([Motor Frequency] (OFR))
  ramp_out = 131,                                 ///< Ramp output ([Ramp Out.] (ORP))
  motor_torque = 132,                             ///< Motor torque ([Motor Torq.] (TRQ))
  signed_torque = 133,                            ///< Signed torque ([Sign. Torque] (STQ))
  signed_ramp = 134,                              ///< Signed ramp ([sign Ramp] (ORS))
  pid_reference = 135,                            ///< PID reference ([PID Ref.] (OPS))
  pid_feedback = 136,                             ///< PID feedback ([PID Feedbk] (OPF))
  pid_error = 137,                                ///< PID error  ([PID Error] (OPE))
  pid_output = 138,                               ///< PID output ([PID Output] (OPI))
  motor_power = 139,                              ///< Motor power  ([Motor Power] (OPR))
  motor_thermal_state = 140,                      ///< Motor thermal state ([Mot Thermal] (THR))
  drive_thermal_state = 141,                      ///< Drive thermal state ([Drv Thermal] (THD))
  reference_frequency_via_di = 160,               ///< Reference frequency via DI ([Ref Frequency via DI] (UPDT))
  reference_frequency_via_remote_terminal = 163,  ///< Reference frequency via remote terminal ([Ref.Freq-Rmt.Term] (LCC))
  reference_frequency_via_modbus = 164,           ///< Reference frequency via Modbus ([Ref. Freq-Modbus] (MDB))
  reference_frequency_via_canopen = 167,          ///< Reference frequency via CANopen ([Ref. Freq-CANopen] (CAN))
  reference_frequency_via_com_module = 169,       ///< Reference frequency via Com module ([Ref. Freq-Com. Module] (NET))
  embedded_ethernet = 171,                        ///< Embedded Ethernet ([Embedded Ethernet] (ETH))
  signed_output_frequency = 173,                  ///< Signed output frequency ([Sig. O/P Frq.] (OFS))
  motor_voltage = 180,                            ///< Motor voltage ([Motor volt.] (UOP))
  ai_virtual_1 = 183,                             ///< AI Virtual 1 ([AI Virtual 1] (AIV1))
  ai_virtual_2 = 185,                             ///< AI Virtual 2 ([AI Virtual 2] (AIV2))
  ai_virtual_3 = 197,                             ///< AI Virtual 3 ([AI Virtual 3] (AIV3))
  di5_pulse_input_assignment = 186,               ///< DI5 pulseInput assignment ([DI5 PulseInput Assignment] (PI5))
  di6_pulse_input_assignment = 187,               ///< DI6 pulseInput assignment ([DI6 PulseInput Assignment] (PI6))
  estimated_pump_flow = 340,                      ///< Estimated pump flow ([Est. Pump Flow] (SLPF))
  inlet_pressure = 341,                           ///< Inlet pressure value ([Inlet Pressure Value] (PS1V))
  outlet_pressure = 342,                          ///< Outlet pressure value ([Outlet Pressure Value] (PS2V))
  installation_flow = 343,                        ///< Installation flow value ([Installation Flow] (FS1V))
  estimated_pump_system_flow = 346,               ///< Estimated pump system flow ([Est. System Flow] (SLSF))
  estimated_pump_head = 347,                      ///< Estimated pump head ([Est. Pump Head] (SLPH))
  estimated_pump_delta_pressure = 348,            ///< Estimated pump delta pressure ([Est. Pump dP] (SLDP))
  estimated_pump_system_delta_pressure = 349,     ///< Estimated pump system delta pressure ([Est. System dP] (SLSD))
  none = 510,                                     ///<  ([None] (MVCO))
};
[[nodiscard]] constexpr auto enum_desc(psa_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case psa_e::not_configured:
      return "[Not Configured] (NO), Not configured";
    case psa_e::analog_input_1:
      return "[AI1] (AI1), AI1 Analog input";
    case psa_e::analog_input_2:
      return "[AI2] (AI2), AI2 Analog input";
    case psa_e::analog_input_3:
      return "[AI3] (AI3), AI3 Analog input";
    case psa_e::analog_input_4:
      return "[AI4] (AI4), AI4 Analog input";
    case psa_e::analog_input_5:
      return "[AI5] (AI5), AI5 Analog input";
    case psa_e::motor_current:
      return "[Motor Current] (OCR), Motor current";
    case psa_e::motor_frequency:
      return "[Motor Frequency] (OFR), Motor frequency";
    case psa_e::ramp_out:
      return "[Ramp Out.] (ORP), Ramp output";
    case psa_e::motor_torque:
      return "[Motor Torq.] (TRQ), Motor torque";
    case psa_e::signed_torque:
      return "[Sign. Torque] (STQ), Signed torque";
    case psa_e::signed_ramp:
      return "[sign Ramp] (ORS), Signed ramp";
    case psa_e::pid_reference:
      return "[PID Ref.] (OPS), PID reference";
    case psa_e::pid_feedback:
      return "[PID Feedbk] (OPF), PID feedback";
    case psa_e::pid_error:
      return "[PID Error] (OPE), PID error ";
    case psa_e::pid_output:
      return "[PID Output] (OPI), PID output";
    case psa_e::motor_power:
      return "[Motor Power] (OPR), Motor power ";
    case psa_e::motor_thermal_state:
      return "[Mot Thermal] (THR), Motor thermal state";
    case psa_e::drive_thermal_state:
      return "[Drv Thermal] (THD), Drive thermal state";
    case psa_e::reference_frequency_via_di:
      return "[Ref Frequency via DI] (UPDT), Reference frequency via DI";
    case psa_e::reference_frequency_via_remote_terminal:
      return "[Ref.Freq-Rmt.Term] (LCC), Reference frequency via remote terminal";
    case psa_e::reference_frequency_via_modbus:
      return "[Ref. Freq-Modbus] (MDB), Reference frequency via Modbus";
    case psa_e::reference_frequency_via_canopen:
      return "[Ref. Freq-CANopen] (CAN), Reference frequency via CANopen";
    case psa_e::reference_frequency_via_com_module:
      return "[Ref. Freq-Com. Module] (NET), Reference frequency via Com module";
    case psa_e::embedded_ethernet:
      return "[Embedded Ethernet] (ETH), Embedded Ethernet";
    case psa_e::signed_output_frequency:
      return "[Sig. O/P Frq.] (OFS), Signed output frequency";
    case psa_e::motor_voltage:
      return "[Motor volt.] (UOP), Motor voltage";
    case psa_e::ai_virtual_1:
      return "[AI Virtual 1] (AIV1), AI Virtual 1";
    case psa_e::ai_virtual_2:
      return "[AI Virtual 2] (AIV2), AI Virtual 2";
    case psa_e::ai_virtual_3:
      return "[AI Virtual 3] (AIV3), AI Virtual 3";
    case psa_e::di5_pulse_input_assignment:
      return "[DI5 PulseInput Assignment] (PI5), DI5 pulseInput assignment";
    case psa_e::di6_pulse_input_assignment:
      return "[DI6 PulseInput Assignment] (PI6), DI6 pulseInput assignment";
    case psa_e::estimated_pump_flow:
      return "[Est. Pump Flow] (SLPF), Estimated pump flow";
    case psa_e::inlet_pressure:
      return "[Inlet Pressure Value] (PS1V), Inlet pressure value";
    case psa_e::outlet_pressure:
      return "[Outlet Pressure Value] (PS2V), Outlet pressure value";
    case psa_e::installation_flow:
      return "[Installation Flow] (FS1V), Installation flow value";
    case psa_e::estimated_pump_system_flow:
      return "[Est. System Flow] (SLSF), Estimated pump system flow";
    case psa_e::estimated_pump_head:
      return "[Est. Pump Head] (SLPH), Estimated pump head";
    case psa_e::estimated_pump_delta_pressure:
      return "[Est. Pump dP] (SLDP), Estimated pump delta pressure";
    case psa_e::estimated_pump_system_delta_pressure:
      return "[Est. System dP] (SLSD), Estimated pump system delta pressure";
    case psa_e::none:
      return "[None] (MVCO), ";
  }
  return "unknown";
}
constexpr auto format_as(psa_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of psl_e enum decleration
enum struct psl_e : std::uint16_t {
  not_assigned = 0,                               ///< Not assigned ([Not Assigned] (NO))
  drive_in_operating_state_fault = 1,             ///< Drive in operating state "Fault" ([Operating State Fault] (FLT))
  drive_running = 2,                              ///< Drive running ([Drive Running] (RUN))
  ouput_contactor_control = 3,                    ///< Ouput contactor control ([Output cont] (OCC))
  motor_frequency_high_threshold_reached = 4,     ///< Motor frequency high threshold reached ([Mot Freq High Thd] (FTA))
  high_speed_reached = 5,                         ///< High speed reached ([High Speed Reached] (FLA))
  current_threshold_reached = 6,                  ///< Current threshold reached ([Current Thd Reached] (CTA))
  reference_frequency_reached = 7,                ///< Reference frequency  reached ([Ref Freq Reached] (SRA))
  motor_thermal_threshold_reached = 8,            ///< Motor thermal threshold reached ([Motor Therm Thd reached] (TSA))
  pid_error_warning = 10,                         ///< PID error warning ([PID error Warning] (PEE))
  pid_feedback_warning = 11,                      ///< PID feedback warning ([PID Feedback Warn] (PFA))
  ai2_4_20_loss_warning = 12,                     ///< AI2 4-20 Loss warning ([AI2 4-20 Loss Warning] (AP2))
  motor_frequency_high_threshold_2_reached = 13,  ///< Motor frequency high threshold 2 reached ([Mot Freq High Thd 2] (F2A))
  drive_thermal_threshold_reached = 14,           ///< Drive thermal threshold reached ([Drv Therm Thd reached] (TAD))
  reference_frequency_high_threshold_reached =
      16,  ///< Reference frequency high threshold reached ([Ref Freq High Thd reached] (RTAH))
  reference_frequency_low_threshold_reached =
      17,  ///< Reference frequency low threshold reached ([Ref Freq Low Thd reached] (RTAL))
  motor_frequency_low_threshold_reached = 18,     ///< Motor frequency low threshold reached ([Mot Freq Low Thd] (FTAL))
  motor_frequency_low_threshold_2_reached = 19,   ///< Motor frequency low threshold 2 reached ([Mot Freq Low Thd 2] (F2AL))
  low_current_threshold_reached = 20,             ///< Low current threshold reached ([Low Current Reached] (CTAL))
  process_underload_warning = 21,                 ///< Process underload warning ([Process Undld Warning] (ULA))
  process_overload_warning = 22,                  ///< Process overload warning ([Process Overload Warning] (OLA))
  pid_high_feedback_warning = 23,                 ///< PID high feedback warning ([PID High Fdbck Warn] (PFAH))
  pid_low_feedback_warning = 24,                  ///< PID low feedback warning ([PID Low Fdbck Warn] (PFAL))
  regulation_warning = 25,                        ///< Regulation warning ([Regulation Warning] (PISH))
  forced_run = 26,                                ///< Forced Run ([Forced Run] (ERN))
  high_torque_warning = 28,                       ///< High torque warning ([High Torque Warning] (TTHA))
  low_torque_warning = 29,                        ///< Low torque warning ([Low Torque Warning] (TTLA))
  run_forward = 30,                               ///< Run forward ([Forward] (MFRD))
  run_reverse = 31,                               ///< Run reverse ([Reverse] (MRRS))
  ramp_switching = 34,                            ///< Ramp switching ([Ramp switching] (RP2))
  hmi_command = 42,                               ///< HMI command ([HMI cmd] (BMP))
  negative_torque = 47,                           ///< Negative torque ([Neg Torque] (ATS))
  configuration_0_active = 48,                    ///< Configuration 0 active ([Cnfg.0 act.] (CNF0))
  parameter_set_1_active = 52,                    ///< Parameter set 1 active ([set 1 active] (CFP1))
  parameter_set_2_active = 53,                    ///< Parameter set 2 active ([set 2 active] (CFP2))
  parameter_set_3_active = 54,                    ///< Parameter set 3 active ([set 3 active] (CFP3))
  parameter_set_4_active = 55,                    ///< Parameter set 4 active ([set 4 active] (CFP4))
  dc_bus_charged = 64,                            ///< DC bus charged ([DC charged] (DBL))
  power_removal_state = 66,                       ///< Power Removal state ([Power Removal State] (PRM))
  mains_contactor_control = 73,                   ///< Mains contactor control ([Mains Contactor] (LLC))
  i_present = 77,                                 ///< I present ([I present] (MCP))
  warning_group_1 = 80,                           ///< Warning group 1 ([Warning Grp 1] (AG1))
  warning_group_2 = 81,                           ///< Warning group 2 ([Warning Grp 2] (AG2))
  warning_group_3 = 82,                           ///< Warning group 3 ([Warning Grp 3] (AG3))
  external_error_warning = 87,                    ///< External error warning ([External Error Warning] (EFA))
  undervoltage_warning = 88,                      ///< Undervoltage warning  ([Undervoltage Warning ] (USA))
  preventive_undervoltage_active = 89,            ///< Preventive undervoltage active ([Preventive UnderV Active] (UPA))
  drive_thermal_state_warning = 91,               ///< Drive thermal state warning ([Drive Thermal Warning] (THA))
  afe_mains_undervoltage = 92,                    ///< AFE Mains undervoltage  ([AFE Mains Undervoltage ] (URA))
  reference_frequency_channel_1 = 96,             ///< Reference frequency channel 1 ([Ref Freq Channel 1] (FR1))
  reference_frequency_channel_2 = 97,             ///< Reference frequency channel 2 ([Ref Freq Channel 2] (FR2))
  command_channel_1 = 98,                         ///< Command channel 1 ([Cmd Channel 1] (CD1))
  command_channel_2 = 99,                         ///< Command channel 1 ([Cmd Channel 2] (CD2))
  command_ch_1b = 100,                            ///< Command ch = ch 1B ([ch1B active] (FR1B))
  igbt_thermal_warning = 104,                     ///< IGBT thermal warning ([IGBT Thermal Warning] (TJA))
  ai3_4_20_loss_warning = 107,                    ///< AI3 4-20 Loss warning ([AI3 4-20 Loss Warning] (AP3))
  ai4_4_20_loss_warning = 108,                    ///< AI4 4-20 Loss warning ([AI4 4-20 Loss Warn] (AP4))
  flow_limit_active = 110,                        ///< Flow limit active ([Flow Limit Active] (FSA))
  graphic_display_terminal_function_key_1 = 116,  ///< Graphic display terminal function key 1 ([Function key 1] (FN1))
  graphic_display_terminal_function_key_2 = 117,  ///< Graphic display terminal function key 2 ([Function key 2] (FN2))
  graphic_display_terminal_function_key_3 = 118,  ///< Graphic display terminal function key 3 ([Function key 3] (FN3))
  graphic_display_terminal_function_key_4 = 119,  ///< Graphic display terminal function key 4 ([Function key 4] (FN4))
  ai1_4_20_loss_warning = 123,                    ///< AI1 4-20 loss warning ([AI1 4-20 Loss Warning] (AP1))
  ready = 127,                                    ///< Ready ([Ready] (RDY))
  yes = 128,                                      ///< Yes ([Yes] (YES))
  digital_input_1 = 129,                          ///< Digital input 1 ([DI1] (LI1))
  digital_input_2 = 130,                          ///< Digital input 2 ([DI2] (LI2))
  digital_input_3 = 131,                          ///< Digital input 3 ([DI3] (LI3))
  digital_input_4 = 132,                          ///< Digital input 4 ([DI4] (LI4))
  digital_input_5 = 133,                          ///< Digital input 5 ([DI5] (LI5))
  digital_input_6 = 134,                          ///< Digital input 6 ([DI6] (LI6))
  digital_input_11 = 139,                         ///< Digital input 11 ([DI11] (LI11))
  digital_input_12 = 140,                         ///< Digital input 12 ([DI12] (LI12))
  digital_input_13 = 141,                         ///< Digital input 13 ([DI13] (LI13))
  digital_input_14 = 142,                         ///< Digital input 14 ([DI14] (LI14))
  digital_input_15 = 143,                         ///< Digital input 15 ([DI15] (LI15))
  digital_input_16 = 144,                         ///< Digital input 16 ([DI16] (LI16))
  bit_0_digital_input_ctrl_word = 160,            ///< Bit 0 digital input ctrl word ([CD00] (CD00))
  bit_1_digital_input_ctrl_word = 161,            ///< Bit 1 digital input ctrl word ([CD01] (CD01))
  bit_2_digital_input_ctrl_word = 162,            ///< Bit 2 digital input ctrl word ([CD02] (CD02))
  bit_3_digital_input_ctrl_word = 163,            ///< Bit 3 digital input ctrl word ([CD03] (CD03))
  bit_4_digital_input_ctrl_word = 164,            ///< Bit 4 digital input ctrl word ([CD04] (CD04))
  bit_5_digital_input_ctrl_word = 165,            ///< Bit 5 digital input ctrl word ([CD05] (CD05))
  bit_6_digital_input_ctrl_word = 166,            ///< Bit 6 digital input ctrl word ([CD06] (CD06))
  bit_7_digital_input_ctrl_word = 167,            ///< Bit 7 digital input ctrl word ([CD07] (CD07))
  bit_8_digital_input_ctrl_word = 168,            ///< Bit 8 digital input ctrl word ([CD08] (CD08))
  bit_9_digital_input_ctrl_word = 169,            ///< Bit 9 digital input ctrl word ([CD09] (CD09))
  bit_10_digital_input_ctrl_word = 170,           ///< Bit10 digital input ctrl word ([CD10] (CD10))
  bit_11_digital_input_ctrl_word = 171,           ///< Bit11 digital input ctrl word ([CD11] (CD11))
  bit_12_digital_input_ctrl_word = 172,           ///< Bit12 digital input ctrl word ([CD12] (CD12))
  bit_13_digital_input_ctrl_word = 173,           ///< Bit13 digital input ctrl word ([CD13] (CD13))
  bit_14_digital_input_ctrl_word = 174,           ///< Bit14 digital input ctrl word ([CD14] (CD14))
  bit_15_digital_input_ctrl_word = 175,           ///< Bit15 digital input ctrl word ([CD15] (CD15))
  bit_0_modbus_ctrl_word = 176,                   ///< Bit 0 Modbus ctrl word ([C100] (C100))
  bit_1_modbus_ctrl_word = 177,                   ///< Bit 1 Modbus ctrl word ([C101] (C101))
  bit_2_modbus_ctrl_word = 178,                   ///< Bit 2 Modbus ctrl word ([C102] (C102))
  bit_3_modbus_ctrl_word = 179,                   ///< Bit 3 Modbus ctrl word ([C103] (C103))
  bit_4_modbus_ctrl_word = 180,                   ///< Bit 4 Modbus ctrl word ([C104] (C104))
  bit_5_modbus_ctrl_word = 181,                   ///< Bit 5 Modbus ctrl word ([C105] (C105))
  bit_6_modbus_ctrl_word = 182,                   ///< Bit 6 Modbus ctrl word ([C106] (C106))
  bit_7_modbus_ctrl_word = 183,                   ///< Bit 7 Modbus ctrl word ([C107] (C107))
  bit_8_modbus_ctrl_word = 184,                   ///< Bit 8 Modbus ctrl word ([C108] (C108))
  bit_9_modbus_ctrl_word = 185,                   ///< Bit 9 Modbus ctrl word ([C109] (C109))
  bit_10_modbus_ctrl_word = 186,                  ///< Bit 10 Modbus ctrl word ([C110] (C110))
  bit_11_modbus_ctrl_word = 187,                  ///< Bit 11 Modbus ctrl word ([C111] (C111))
  bit_12_modbus_ctrl_word = 188,                  ///< Bit 12 Modbus ctrl word ([C112] (C112))
  bit_13_modbus_ctrl_word = 189,                  ///< Bit 13 Modbus ctrl word ([C113] (C113))
  bit_14_modbus_ctrl_word = 190,                  ///< Bit 14 Modbus ctrl word ([C114] (C114))
  bit_15_modbus_ctrl_word = 191,                  ///< Bit 15 Modbus ctrl word ([C115] (C115))
  bit_0_canopen_ctrl_word = 192,                  ///< Bit 0 CANopen ctrl word ([C200] (C200))
  bit_1_canopen_ctrl_word = 193,                  ///< Bit 1 CANopen ctrl word ([C201] (C201))
  bit_2_canopen_ctrl_word = 194,                  ///< Bit 2 CANopen ctrl word ([C202] (C202))
  bit_3_canopen_ctrl_word = 195,                  ///< Bit 3 CANopen ctrl word ([C203] (C203))
  bit_4_canopen_ctrl_word = 196,                  ///< Bit 4 CANopen ctrl word ([C204] (C204))
  bit_5_canopen_ctrl_word = 197,                  ///< Bit 5 CANopen ctrl word ([C205] (C205))
  bit_6_canopen_ctrl_word = 198,                  ///< Bit 6 CANopen ctrl word ([C206] (C206))
  bit_7_canopen_ctrl_word = 199,                  ///< Bit 7 CANopen ctrl word ([C207] (C207))
  bit_8_canopen_ctrl_word = 200,                  ///< Bit 8 CANopen ctrl word ([C208] (C208))
  bit_9_canopen_ctrl_word = 201,                  ///< Bit 9 CANopen ctrl word ([C209] (C209))
  bit_10_canopen_ctrl_word = 202,                 ///< Bit 10 CANopen ctrl word ([C210] (C210))
  bit_11_canopen_ctrl_word = 203,                 ///< Bit 11 CANopen ctrl word ([C211] (C211))
  bit_12_canopen_ctrl_word = 204,                 ///< Bit 12 CANopen ctrl word ([C212] (C212))
  bit_13_canopen_ctrl_word = 205,                 ///< Bit 13 CANopen ctrl word ([C213] (C213))
  bit_14_canopen_ctrl_word = 206,                 ///< Bit 14 CANopen ctrl word ([C214] (C214))
  bit_15_canopen_ctrl_word = 207,                 ///< Bit 15 CANopen ctrl word ([C215] (C215))
  bit_0_com_module_ctrl_word = 208,               ///< Bit 0 Com module ctrl word ([C300] (C300))
  bit_1_com_module_ctrl_word = 209,               ///< Bit 1 Com module ctrl word ([C301] (C301))
  bit_2_com_module_ctrl_word = 210,               ///< Bit 2 Com module ctrl word ([C302] (C302))
  bit_3_com_module_ctrl_word = 211,               ///< Bit 3 Com module ctrl word ([C303] (C303))
  bit_4_com_module_ctrl_word = 212,               ///< Bit 4 Com module ctrl word ([C304] (C304))
  bit_5_com_module_ctrl_word = 213,               ///< Bit 5 Com module ctrl word ([C305] (C305))
  bit_6_com_module_ctrl_word = 214,               ///< Bit 6 Com module ctrl word ([C306] (C306))
  bit_7_com_module_ctrl_word = 215,               ///< Bit 7 Com module ctrl word ([C307] (C307))
  bit_8_com_module_ctrl_word = 216,               ///< Bit 8 Com module ctrl word ([C308] (C308))
  bit_9_com_module_ctrl_word = 217,               ///< Bit 9 Com module ctrl word ([C309] (C309))
  bit_10_com_module_ctrl_word = 218,              ///< Bit 10 Com module ctrl word ([C310] (C310))
  bit_11_com_module_ctrl_word = 219,              ///< Bit 11 Com module ctrl word ([C311] (C311))
  bit_12_com_module_ctrl_word = 220,              ///< Bit 12 Com module ctrl word ([C312] (C312))
  bit_13_com_module_ctrl_word = 221,              ///< Bit 13 Com module ctrl word ([C313] (C313))
  bit_14_com_module_ctrl_word = 222,              ///< Bit 14 Com module ctrl word ([C314] (C314))
  bit_15_com_module_ctrl_word = 223,              ///< Bit 15 Com module ctrl word ([C315] (C315))
  c500 = 240,                                     ///< C500 ([C500] (C500))
  c501 = 241,                                     ///< C501 ([C501] (C501))
  c502 = 242,                                     ///< C502 ([C502] (C502))
  c503 = 243,                                     ///< C503 ([C503] (C503))
  c504 = 244,                                     ///< C504 ([C504] (C504))
  c505 = 245,                                     ///< C505 ([C505] (C505))
  c506 = 246,                                     ///< C506 ([C506] (C506))
  c507 = 247,                                     ///< C507 ([C507] (C507))
  c508 = 248,                                     ///< C508 ([C508] (C508))
  c509 = 249,                                     ///< C509 ([C509] (C509))
  c510 = 250,                                     ///< C510 ([C510] (C510))
  c511 = 251,                                     ///< C511 ([C511] (C511))
  c512 = 252,                                     ///< C512 ([C512] (C512))
  c513 = 253,                                     ///< C513 ([C513] (C513))
  c514 = 254,                                     ///< C514 ([C514] (C514))
  c515 = 255,                                     ///< C515 ([C515] (C515))
  digital_input_di1_low_level = 272,              ///< Digital input DI1 (low level) ([DI1 (Low level)] (L1L))
  digital_input_di2_low_level = 273,              ///< Digital input DI2 (low level) ([DI2 (Low level)] (L2L))
  digital_input_di3_low_level = 274,              ///< Digital input DI3 (low level) ([DI3 (Low level)] (L3L))
  digital_input_di4_low_level = 275,              ///< Digital input DI4 (low level) ([DI4 (Low level)] (L4L))
  digital_input_di5_low_level = 276,              ///< Digital input DI5 (low level) ([DI5 (Low level)] (L5L))
  digital_input_di6_low_level = 277,              ///< Digital input DI6 (low level) ([DI6 (Low level)] (L6L))
  digital_input_di11_low_level = 282,             ///< Digital input DI1 (low level) ([DI11 (Low level)] (L11L))
  digital_input_di12_low_level = 283,             ///< Digital input DI12 (low level) ([DI12 (Low level)] (L12L))
  digital_input_di13_low_level = 284,             ///< Digital input DI13 (low level) ([DI13 (Low level)] (L13L))
  digital_input_di14_low_level = 285,             ///< Digital input DI14 (low level) ([DI14 (Low level)] (L14L))
  digital_input_di15_low_level = 286,             ///< Digital input DI15 (low level) ([DI15 (Low level)] (L15L))
  digital_input_di16_low_level = 287,             ///< Digital input DI16 (low level) ([DI16 (Low level)] (L16L))
  digital_input_di50_high_level = 302,            ///< Digital input DI50 (High level) ([DI50 (High Level)] (D50H))
  digital_input_di51_high_level = 303,            ///< Digital input DI51 (High level) ([DI51 (High Level)] (D51H))
  digital_input_di52_high_level = 304,            ///< Digital input DI52 (High level) ([DI52 (High Level)] (D52H))
  digital_input_di53_high_level = 305,            ///< Digital input DI53 (High level) ([DI53 (High Level)] (D53H))
  digital_input_di54_high_level = 306,            ///< Digital input DI54 (High level) ([DI54 (High Level)] (D54H))
  digital_input_di55_high_level = 307,            ///< Digital input DI55 (High level) ([DI55 (High Level)] (D55H))
  digital_input_di56_high_level = 308,            ///< Digital input DI56 (High level) ([DI56 (High Level)] (D56H))
  digital_input_di57_high_level = 309,            ///< Digital input DI57 (High level) ([DI57 (High Level)] (D57H))
  digital_input_di58_high_level = 310,            ///< Digital input DI58 (High level) ([DI58 (High Level)] (D58H))
  digital_input_di59_high_level = 311,            ///< Digital input DI59 (High level) ([DI59 (High Level)] (D59H))
  digital_input_di50_low_level = 312,             ///< Digital input DI50 (low level) ([DI50 (Low level)] (D50L))
  digital_input_di51_low_level = 313,             ///< Digital input DI51 (low level) ([DI51 (Low level)] (D51L))
  digital_input_di52_low_level = 314,             ///< Digital input DI52 (low level) ([DI52 (Low level)] (D52L))
  digital_input_di53_low_level = 315,             ///< Digital input DI53 (low level) ([DI53 (Low level)] (D53L))
  digital_input_di54_low_level = 316,             ///< Digital input DI54 (low level) ([DI54 (Low level)] (D54L))
  digital_input_di55_low_level = 317,             ///< Digital input DI55 (low level) ([DI55 (Low level)] (D55L))
  digital_input_di56_low_level = 318,             ///< Digital input DI56 (low level) ([DI56 (Low level)] (D56L))
  digital_input_di57_low_level = 319,             ///< Digital input DI57 (low level) ([DI57 (Low level)] (D57L))
  digital_input_di58_low_level = 320,             ///< Digital input DI58 (low level) ([DI58 (Low level)] (D58L))
  digital_input_di59_low_level = 321,             ///< Digital input DI59 (low level) ([DI59 (Low level)] (D59L))
  dc_bus_ripple_warning = 336,                    ///< DC bus ripple warning ([DC Bus Ripple Warn] (DCRW))
  jockey = 340,                                   ///< Jockey ([Jockey] (JOKY))
  priming = 341,                                  ///< Priming ([Priming] (PRIM))
  anti_jam_active = 342,                          ///< Anti-Jam active ([Anti-Jam Active] (JAMR))
  pipe_fill = 344,                                ///< Pipe Fill ([Pipe Fill] (FILL))
  priming_pump_active = 345,                      ///< Priming pump active ([Priming Pump Active] (PPON))
  dry_run_warning = 346,                          ///< Dry run warning ([Dry Run Warning] (DRYA))
  pump_low_flow = 347,                            ///< Pump low flow  ([Pump Low Flow ] (PLFA))
  process_high_flow_warning = 348,                ///< Process high flow warning ([Proc High Flow Warn] (HFPA))
  inlet_pressure_warning = 349,                   ///< Inlet pressure warning ([InPress Warning] (IPPA))
  outlet_pressure_low_warning = 350,              ///< Outlet pressure low warning ([Low OutPres Warning] (OPLA))
  outlet_pressure_high_warning = 351,             ///< Outlet pressure high warning ([High OutPres Warn] (OPHA))
  pump_cycle_warning = 352,                       ///< Pump cycle warning ([Pump Cycle Warning] (PCPA))
  anti_jam_warning = 353,                         ///< Anti-Jam warning ([Anti-Jam Warning] (JAMA))
  low_flow_warning = 354,                         ///< Low flow warning ([Low Flow Warning] (LFA))
  low_pressure_warning = 355,                     ///< Low pressure warning ([Low Pressure Warn] (LPA))
  output_pressure_high_switch_warning = 356,      ///< Output pressure high switch warning ([Switch OutPres Warn] (OPSA))
  jockey_pump_active = 357,                       ///< Jockey pump active ([Jockey Pump Active] (JPON))
  pump_1_command = 358,                           ///< Pump 1 command ([Pump 1 Cmd] (MPO1))
  pump_2_command = 359,                           ///< Pump 2 command ([Pump 2 Cmd] (MPO2))
  pump_3_command = 360,                           ///< Pump 3 command ([Pump 3 Cmd] (MPO3))
  pump_4_command = 361,                           ///< Pump 4 command ([Pump 4 Cmd] (MPO4))
  pump_5_command = 362,                           ///< Pump 5 command ([Pump 5 Cmd] (MPO5))
  pump_6_command = 363,                           ///< Pump 6 command ([Pump 6 Cmd] (MPO6))
  multi_pump_available_capacity_warning = 364,    ///< Multi-Pump available capacity warning ([MP Capacity Warn] (MPCA))
  lead_pump_not_available = 365,                  ///< Lead pump not available ([Lead Pump Warn] (MPLA))
  high_level_warning = 366,                       ///< High level warning ([High Level Warning] (LCHA))
  low_level_warning = 367,                        ///< Low level warning ([Low Level Warning] (LCLA))
  level_switch_warning = 368,                     ///< Level switch warning ([Level Switch Warning] (LCWA))
  multipump_device_warning = 369,                 ///< Multipump device warning ([M/P Device Warn] (MPDA))
  multi_pump_master_activated = 370,              ///< Multi-pump master activated ([M/P Master Activated] (MPMA))
  temperature_sensor_ai2_warning = 475,           ///< Temperature sensor AI2 warning ([Temp Sens AI2 Warn] (TS2A))
  temperature_sensor_ai3_warning = 476,           ///< Temperature sensor AI3 warning ([Temp Sens AI3 Warn] (TS3A))
  temperature_sensor_ai4_warning = 477,           ///< Temperature sensor AI4 warning ([Temp Sens AI4 Warn] (TS4A))
  temperature_sensor_ai5_warning = 478,           ///< Temperature sensor AI5 warning ([Temp Sens AI5 Warn] (TS5A))
  customer_warning_5 = 484,                       ///< Customer warning 5 ([Cust Warning 5] (CAS5))
  cabinet_fan_command = 488,                      ///< Cabinet fan command ([Cabinet Fan Command] (FCC))
  circuit_breaker_start_pulse = 489,              ///< Circuit breaker start pulse ([CB Start Pulse] (CBEP))
  circuit_breaker_stop_pulse = 490,               ///< Circuit breaker stop pulse ([CB Stop Pulse] (CBDP))
  power_consumption_warning = 491,                ///< Power Consumption warning ([Power Cons Warning] (POWD))
  warning_group_4 = 492,                          ///< Warning group 4 ([Warning Grp 4] (AG4))
  warning_group_5 = 493,                          ///< Warning group 5 ([Warning Grp 5] (AG5))
  fallback_speed = 494,                           ///< Fallback speed ([Fallback speed] (FRF))
  speed_maintained = 495,                         ///< Speed maintained ([Speed Maintained] (RLS))
  per_type_of_stop = 496,                         ///< Per type of stop ([Per Type of Stop] (STT))
  life_cycle_warning_1 = 497,                     ///< Life cycle warning 1 ([Life Cycle Warn 1] (LCA1))
  life_cycle_warning_2 = 498,                     ///< Life cycle warning 2 ([Life Cycle Warn 2] (LCA2))
  ai2_thermal_sensor_warning = 499,               ///< AI2 thermal sensor warning ([AI2 Th Warning] (TP2A))
  ai3_thermal_sensor_warning = 500,               ///< AI3 thermal sensor warning ([AI3 Th Warning] (TP3A))
  ai4_thermal_sensor_warning = 501,               ///< AI4 thermal sensor warning ([AI4 Th Warning] (TP4A))
  ai5_thermal_sensor_warning = 502,               ///< AI5 thermal sensor warning ([AI5 Th Warning] (TP5A))
  ai5_4_20_loss_warning = 503,                    ///< AI5 4-20 Loss warning ([AI5 4-20 Loss Warn] (AP5))
  fan_counter_warning = 504,                      ///< Fan counter warning ([Fan Counter Warning] (FCTA))
  fan_feedback_warning = 505,                     ///< Fan feedback warning ([Fan Feedback Warn] (FFDA))
  power_high_threshold = 506,                     ///< Power high threshold ([Power High Threshold] (PTHA))
  power_low_threshold = 507,                      ///< Power low threshold ([Power Low Threshold] (PTHL))
  customer_warning_1 = 508,                       ///< Customer warning 1 ([Cust Warning 1] (CAS1))
  customer_warning_2 = 509,                       ///< Customer warning 2 ([Cust Warning 2] (CAS2))
  customer_warning_3 = 510,                       ///< Customer warning 3 ([Cust Warning 3] (CAS3))
  customer_warning_4 = 511,                       ///< Customer warning 4 ([Cust Warning 4] (CAS4))
};
[[nodiscard]] constexpr auto enum_desc(psl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case psl_e::not_assigned:
      return "[Not Assigned] (NO), Not assigned";
    case psl_e::drive_in_operating_state_fault:
      return "[Operating State Fault] (FLT), Drive in operating state Fault";
    case psl_e::drive_running:
      return "[Drive Running] (RUN), Drive running";
    case psl_e::ouput_contactor_control:
      return "[Output cont] (OCC), Ouput contactor control";
    case psl_e::motor_frequency_high_threshold_reached:
      return "[Mot Freq High Thd] (FTA), Motor frequency high threshold reached";
    case psl_e::high_speed_reached:
      return "[High Speed Reached] (FLA), High speed reached";
    case psl_e::current_threshold_reached:
      return "[Current Thd Reached] (CTA), Current threshold reached";
    case psl_e::reference_frequency_reached:
      return "[Ref Freq Reached] (SRA), Reference frequency  reached";
    case psl_e::motor_thermal_threshold_reached:
      return "[Motor Therm Thd reached] (TSA), Motor thermal threshold reached";
    case psl_e::pid_error_warning:
      return "[PID error Warning] (PEE), PID error warning";
    case psl_e::pid_feedback_warning:
      return "[PID Feedback Warn] (PFA), PID feedback warning";
    case psl_e::ai2_4_20_loss_warning:
      return "[AI2 4-20 Loss Warning] (AP2), AI2 4-20 Loss warning";
    case psl_e::motor_frequency_high_threshold_2_reached:
      return "[Mot Freq High Thd 2] (F2A), Motor frequency high threshold 2 reached";
    case psl_e::drive_thermal_threshold_reached:
      return "[Drv Therm Thd reached] (TAD), Drive thermal threshold reached";
    case psl_e::reference_frequency_high_threshold_reached:
      return "[Ref Freq High Thd reached] (RTAH), Reference frequency high threshold reached";
    case psl_e::reference_frequency_low_threshold_reached:
      return "[Ref Freq Low Thd reached] (RTAL), Reference frequency low threshold reached";
    case psl_e::motor_frequency_low_threshold_reached:
      return "[Mot Freq Low Thd] (FTAL), Motor frequency low threshold reached";
    case psl_e::motor_frequency_low_threshold_2_reached:
      return "[Mot Freq Low Thd 2] (F2AL), Motor frequency low threshold 2 reached";
    case psl_e::low_current_threshold_reached:
      return "[Low Current Reached] (CTAL), Low current threshold reached";
    case psl_e::process_underload_warning:
      return "[Process Undld Warning] (ULA), Process underload warning";
    case psl_e::process_overload_warning:
      return "[Process Overload Warning] (OLA), Process overload warning";
    case psl_e::pid_high_feedback_warning:
      return "[PID High Fdbck Warn] (PFAH), PID high feedback warning";
    case psl_e::pid_low_feedback_warning:
      return "[PID Low Fdbck Warn] (PFAL), PID low feedback warning";
    case psl_e::regulation_warning:
      return "[Regulation Warning] (PISH), Regulation warning";
    case psl_e::forced_run:
      return "[Forced Run] (ERN), Forced Run";
    case psl_e::high_torque_warning:
      return "[High Torque Warning] (TTHA), High torque warning";
    case psl_e::low_torque_warning:
      return "[Low Torque Warning] (TTLA), Low torque warning";
    case psl_e::run_forward:
      return "[Forward] (MFRD), Run forward";
    case psl_e::run_reverse:
      return "[Reverse] (MRRS), Run reverse";
    case psl_e::ramp_switching:
      return "[Ramp switching] (RP2), Ramp switching";
    case psl_e::hmi_command:
      return "[HMI cmd] (BMP), HMI command";
    case psl_e::negative_torque:
      return "[Neg Torque] (ATS), Negative torque";
    case psl_e::configuration_0_active:
      return "[Cnfg.0 act.] (CNF0), Configuration 0 active";
    case psl_e::parameter_set_1_active:
      return "[set 1 active] (CFP1), Parameter set 1 active";
    case psl_e::parameter_set_2_active:
      return "[set 2 active] (CFP2), Parameter set 2 active";
    case psl_e::parameter_set_3_active:
      return "[set 3 active] (CFP3), Parameter set 3 active";
    case psl_e::parameter_set_4_active:
      return "[set 4 active] (CFP4), Parameter set 4 active";
    case psl_e::dc_bus_charged:
      return "[DC charged] (DBL), DC bus charged";
    case psl_e::power_removal_state:
      return "[Power Removal State] (PRM), Power Removal state";
    case psl_e::mains_contactor_control:
      return "[Mains Contactor] (LLC), Mains contactor control";
    case psl_e::i_present:
      return "[I present] (MCP), I present";
    case psl_e::warning_group_1:
      return "[Warning Grp 1] (AG1), Warning group 1";
    case psl_e::warning_group_2:
      return "[Warning Grp 2] (AG2), Warning group 2";
    case psl_e::warning_group_3:
      return "[Warning Grp 3] (AG3), Warning group 3";
    case psl_e::external_error_warning:
      return "[External Error Warning] (EFA), External error warning";
    case psl_e::undervoltage_warning:
      return "[Undervoltage Warning ] (USA), Undervoltage warning ";
    case psl_e::preventive_undervoltage_active:
      return "[Preventive UnderV Active] (UPA), Preventive undervoltage active";
    case psl_e::drive_thermal_state_warning:
      return "[Drive Thermal Warning] (THA), Drive thermal state warning";
    case psl_e::afe_mains_undervoltage:
      return "[AFE Mains Undervoltage ] (URA), AFE Mains undervoltage ";
    case psl_e::reference_frequency_channel_1:
      return "[Ref Freq Channel 1] (FR1), Reference frequency channel 1";
    case psl_e::reference_frequency_channel_2:
      return "[Ref Freq Channel 2] (FR2), Reference frequency channel 2";
    case psl_e::command_channel_1:
      return "[Cmd Channel 1] (CD1), Command channel 1";
    case psl_e::command_channel_2:
      return "[Cmd Channel 2] (CD2), Command channel 1";
    case psl_e::command_ch_1b:
      return "[ch1B active] (FR1B), Command ch = ch 1B";
    case psl_e::igbt_thermal_warning:
      return "[IGBT Thermal Warning] (TJA), IGBT thermal warning";
    case psl_e::ai3_4_20_loss_warning:
      return "[AI3 4-20 Loss Warning] (AP3), AI3 4-20 Loss warning";
    case psl_e::ai4_4_20_loss_warning:
      return "[AI4 4-20 Loss Warn] (AP4), AI4 4-20 Loss warning";
    case psl_e::flow_limit_active:
      return "[Flow Limit Active] (FSA), Flow limit active";
    case psl_e::graphic_display_terminal_function_key_1:
      return "[Function key 1] (FN1), Graphic display terminal function key 1";
    case psl_e::graphic_display_terminal_function_key_2:
      return "[Function key 2] (FN2), Graphic display terminal function key 2";
    case psl_e::graphic_display_terminal_function_key_3:
      return "[Function key 3] (FN3), Graphic display terminal function key 3";
    case psl_e::graphic_display_terminal_function_key_4:
      return "[Function key 4] (FN4), Graphic display terminal function key 4";
    case psl_e::ai1_4_20_loss_warning:
      return "[AI1 4-20 Loss Warning] (AP1), AI1 4-20 loss warning";
    case psl_e::ready:
      return "[Ready] (RDY), Ready";
    case psl_e::yes:
      return "[Yes] (YES), Yes";
    case psl_e::digital_input_1:
      return "[DI1] (LI1), Digital input 1";
    case psl_e::digital_input_2:
      return "[DI2] (LI2), Digital input 2";
    case psl_e::digital_input_3:
      return "[DI3] (LI3), Digital input 3";
    case psl_e::digital_input_4:
      return "[DI4] (LI4), Digital input 4";
    case psl_e::digital_input_5:
      return "[DI5] (LI5), Digital input 5";
    case psl_e::digital_input_6:
      return "[DI6] (LI6), Digital input 6";
    case psl_e::digital_input_11:
      return "[DI11] (LI11), Digital input 11";
    case psl_e::digital_input_12:
      return "[DI12] (LI12), Digital input 12";
    case psl_e::digital_input_13:
      return "[DI13] (LI13), Digital input 13";
    case psl_e::digital_input_14:
      return "[DI14] (LI14), Digital input 14";
    case psl_e::digital_input_15:
      return "[DI15] (LI15), Digital input 15";
    case psl_e::digital_input_16:
      return "[DI16] (LI16), Digital input 16";
    case psl_e::bit_0_digital_input_ctrl_word:
      return "[CD00] (CD00), Bit 0 digital input ctrl word";
    case psl_e::bit_1_digital_input_ctrl_word:
      return "[CD01] (CD01), Bit 1 digital input ctrl word";
    case psl_e::bit_2_digital_input_ctrl_word:
      return "[CD02] (CD02), Bit 2 digital input ctrl word";
    case psl_e::bit_3_digital_input_ctrl_word:
      return "[CD03] (CD03), Bit 3 digital input ctrl word";
    case psl_e::bit_4_digital_input_ctrl_word:
      return "[CD04] (CD04), Bit 4 digital input ctrl word";
    case psl_e::bit_5_digital_input_ctrl_word:
      return "[CD05] (CD05), Bit 5 digital input ctrl word";
    case psl_e::bit_6_digital_input_ctrl_word:
      return "[CD06] (CD06), Bit 6 digital input ctrl word";
    case psl_e::bit_7_digital_input_ctrl_word:
      return "[CD07] (CD07), Bit 7 digital input ctrl word";
    case psl_e::bit_8_digital_input_ctrl_word:
      return "[CD08] (CD08), Bit 8 digital input ctrl word";
    case psl_e::bit_9_digital_input_ctrl_word:
      return "[CD09] (CD09), Bit 9 digital input ctrl word";
    case psl_e::bit_10_digital_input_ctrl_word:
      return "[CD10] (CD10), Bit10 digital input ctrl word";
    case psl_e::bit_11_digital_input_ctrl_word:
      return "[CD11] (CD11), Bit11 digital input ctrl word";
    case psl_e::bit_12_digital_input_ctrl_word:
      return "[CD12] (CD12), Bit12 digital input ctrl word";
    case psl_e::bit_13_digital_input_ctrl_word:
      return "[CD13] (CD13), Bit13 digital input ctrl word";
    case psl_e::bit_14_digital_input_ctrl_word:
      return "[CD14] (CD14), Bit14 digital input ctrl word";
    case psl_e::bit_15_digital_input_ctrl_word:
      return "[CD15] (CD15), Bit15 digital input ctrl word";
    case psl_e::bit_0_modbus_ctrl_word:
      return "[C100] (C100), Bit 0 Modbus ctrl word";
    case psl_e::bit_1_modbus_ctrl_word:
      return "[C101] (C101), Bit 1 Modbus ctrl word";
    case psl_e::bit_2_modbus_ctrl_word:
      return "[C102] (C102), Bit 2 Modbus ctrl word";
    case psl_e::bit_3_modbus_ctrl_word:
      return "[C103] (C103), Bit 3 Modbus ctrl word";
    case psl_e::bit_4_modbus_ctrl_word:
      return "[C104] (C104), Bit 4 Modbus ctrl word";
    case psl_e::bit_5_modbus_ctrl_word:
      return "[C105] (C105), Bit 5 Modbus ctrl word";
    case psl_e::bit_6_modbus_ctrl_word:
      return "[C106] (C106), Bit 6 Modbus ctrl word";
    case psl_e::bit_7_modbus_ctrl_word:
      return "[C107] (C107), Bit 7 Modbus ctrl word";
    case psl_e::bit_8_modbus_ctrl_word:
      return "[C108] (C108), Bit 8 Modbus ctrl word";
    case psl_e::bit_9_modbus_ctrl_word:
      return "[C109] (C109), Bit 9 Modbus ctrl word";
    case psl_e::bit_10_modbus_ctrl_word:
      return "[C110] (C110), Bit 10 Modbus ctrl word";
    case psl_e::bit_11_modbus_ctrl_word:
      return "[C111] (C111), Bit 11 Modbus ctrl word";
    case psl_e::bit_12_modbus_ctrl_word:
      return "[C112] (C112), Bit 12 Modbus ctrl word";
    case psl_e::bit_13_modbus_ctrl_word:
      return "[C113] (C113), Bit 13 Modbus ctrl word";
    case psl_e::bit_14_modbus_ctrl_word:
      return "[C114] (C114), Bit 14 Modbus ctrl word";
    case psl_e::bit_15_modbus_ctrl_word:
      return "[C115] (C115), Bit 15 Modbus ctrl word";
    case psl_e::bit_0_canopen_ctrl_word:
      return "[C200] (C200), Bit 0 CANopen ctrl word";
    case psl_e::bit_1_canopen_ctrl_word:
      return "[C201] (C201), Bit 1 CANopen ctrl word";
    case psl_e::bit_2_canopen_ctrl_word:
      return "[C202] (C202), Bit 2 CANopen ctrl word";
    case psl_e::bit_3_canopen_ctrl_word:
      return "[C203] (C203), Bit 3 CANopen ctrl word";
    case psl_e::bit_4_canopen_ctrl_word:
      return "[C204] (C204), Bit 4 CANopen ctrl word";
    case psl_e::bit_5_canopen_ctrl_word:
      return "[C205] (C205), Bit 5 CANopen ctrl word";
    case psl_e::bit_6_canopen_ctrl_word:
      return "[C206] (C206), Bit 6 CANopen ctrl word";
    case psl_e::bit_7_canopen_ctrl_word:
      return "[C207] (C207), Bit 7 CANopen ctrl word";
    case psl_e::bit_8_canopen_ctrl_word:
      return "[C208] (C208), Bit 8 CANopen ctrl word";
    case psl_e::bit_9_canopen_ctrl_word:
      return "[C209] (C209), Bit 9 CANopen ctrl word";
    case psl_e::bit_10_canopen_ctrl_word:
      return "[C210] (C210), Bit 10 CANopen ctrl word";
    case psl_e::bit_11_canopen_ctrl_word:
      return "[C211] (C211), Bit 11 CANopen ctrl word";
    case psl_e::bit_12_canopen_ctrl_word:
      return "[C212] (C212), Bit 12 CANopen ctrl word";
    case psl_e::bit_13_canopen_ctrl_word:
      return "[C213] (C213), Bit 13 CANopen ctrl word";
    case psl_e::bit_14_canopen_ctrl_word:
      return "[C214] (C214), Bit 14 CANopen ctrl word";
    case psl_e::bit_15_canopen_ctrl_word:
      return "[C215] (C215), Bit 15 CANopen ctrl word";
    case psl_e::bit_0_com_module_ctrl_word:
      return "[C300] (C300), Bit 0 Com module ctrl word";
    case psl_e::bit_1_com_module_ctrl_word:
      return "[C301] (C301), Bit 1 Com module ctrl word";
    case psl_e::bit_2_com_module_ctrl_word:
      return "[C302] (C302), Bit 2 Com module ctrl word";
    case psl_e::bit_3_com_module_ctrl_word:
      return "[C303] (C303), Bit 3 Com module ctrl word";
    case psl_e::bit_4_com_module_ctrl_word:
      return "[C304] (C304), Bit 4 Com module ctrl word";
    case psl_e::bit_5_com_module_ctrl_word:
      return "[C305] (C305), Bit 5 Com module ctrl word";
    case psl_e::bit_6_com_module_ctrl_word:
      return "[C306] (C306), Bit 6 Com module ctrl word";
    case psl_e::bit_7_com_module_ctrl_word:
      return "[C307] (C307), Bit 7 Com module ctrl word";
    case psl_e::bit_8_com_module_ctrl_word:
      return "[C308] (C308), Bit 8 Com module ctrl word";
    case psl_e::bit_9_com_module_ctrl_word:
      return "[C309] (C309), Bit 9 Com module ctrl word";
    case psl_e::bit_10_com_module_ctrl_word:
      return "[C310] (C310), Bit 10 Com module ctrl word";
    case psl_e::bit_11_com_module_ctrl_word:
      return "[C311] (C311), Bit 11 Com module ctrl word";
    case psl_e::bit_12_com_module_ctrl_word:
      return "[C312] (C312), Bit 12 Com module ctrl word";
    case psl_e::bit_13_com_module_ctrl_word:
      return "[C313] (C313), Bit 13 Com module ctrl word";
    case psl_e::bit_14_com_module_ctrl_word:
      return "[C314] (C314), Bit 14 Com module ctrl word";
    case psl_e::bit_15_com_module_ctrl_word:
      return "[C315] (C315), Bit 15 Com module ctrl word";
    case psl_e::c500:
      return "[C500] (C500), C500";
    case psl_e::c501:
      return "[C501] (C501), C501";
    case psl_e::c502:
      return "[C502] (C502), C502";
    case psl_e::c503:
      return "[C503] (C503), C503";
    case psl_e::c504:
      return "[C504] (C504), C504";
    case psl_e::c505:
      return "[C505] (C505), C505";
    case psl_e::c506:
      return "[C506] (C506), C506";
    case psl_e::c507:
      return "[C507] (C507), C507";
    case psl_e::c508:
      return "[C508] (C508), C508";
    case psl_e::c509:
      return "[C509] (C509), C509";
    case psl_e::c510:
      return "[C510] (C510), C510";
    case psl_e::c511:
      return "[C511] (C511), C511";
    case psl_e::c512:
      return "[C512] (C512), C512";
    case psl_e::c513:
      return "[C513] (C513), C513";
    case psl_e::c514:
      return "[C514] (C514), C514";
    case psl_e::c515:
      return "[C515] (C515), C515";
    case psl_e::digital_input_di1_low_level:
      return "[DI1 (Low level)] (L1L), Digital input DI1 (low level)";
    case psl_e::digital_input_di2_low_level:
      return "[DI2 (Low level)] (L2L), Digital input DI2 (low level)";
    case psl_e::digital_input_di3_low_level:
      return "[DI3 (Low level)] (L3L), Digital input DI3 (low level)";
    case psl_e::digital_input_di4_low_level:
      return "[DI4 (Low level)] (L4L), Digital input DI4 (low level)";
    case psl_e::digital_input_di5_low_level:
      return "[DI5 (Low level)] (L5L), Digital input DI5 (low level)";
    case psl_e::digital_input_di6_low_level:
      return "[DI6 (Low level)] (L6L), Digital input DI6 (low level)";
    case psl_e::digital_input_di11_low_level:
      return "[DI11 (Low level)] (L11L), Digital input DI1 (low level)";
    case psl_e::digital_input_di12_low_level:
      return "[DI12 (Low level)] (L12L), Digital input DI12 (low level)";
    case psl_e::digital_input_di13_low_level:
      return "[DI13 (Low level)] (L13L), Digital input DI13 (low level)";
    case psl_e::digital_input_di14_low_level:
      return "[DI14 (Low level)] (L14L), Digital input DI14 (low level)";
    case psl_e::digital_input_di15_low_level:
      return "[DI15 (Low level)] (L15L), Digital input DI15 (low level)";
    case psl_e::digital_input_di16_low_level:
      return "[DI16 (Low level)] (L16L), Digital input DI16 (low level)";
    case psl_e::digital_input_di50_high_level:
      return "[DI50 (High Level)] (D50H), Digital input DI50 (High level)";
    case psl_e::digital_input_di51_high_level:
      return "[DI51 (High Level)] (D51H), Digital input DI51 (High level)";
    case psl_e::digital_input_di52_high_level:
      return "[DI52 (High Level)] (D52H), Digital input DI52 (High level)";
    case psl_e::digital_input_di53_high_level:
      return "[DI53 (High Level)] (D53H), Digital input DI53 (High level)";
    case psl_e::digital_input_di54_high_level:
      return "[DI54 (High Level)] (D54H), Digital input DI54 (High level)";
    case psl_e::digital_input_di55_high_level:
      return "[DI55 (High Level)] (D55H), Digital input DI55 (High level)";
    case psl_e::digital_input_di56_high_level:
      return "[DI56 (High Level)] (D56H), Digital input DI56 (High level)";
    case psl_e::digital_input_di57_high_level:
      return "[DI57 (High Level)] (D57H), Digital input DI57 (High level)";
    case psl_e::digital_input_di58_high_level:
      return "[DI58 (High Level)] (D58H), Digital input DI58 (High level)";
    case psl_e::digital_input_di59_high_level:
      return "[DI59 (High Level)] (D59H), Digital input DI59 (High level)";
    case psl_e::digital_input_di50_low_level:
      return "[DI50 (Low level)] (D50L), Digital input DI50 (low level)";
    case psl_e::digital_input_di51_low_level:
      return "[DI51 (Low level)] (D51L), Digital input DI51 (low level)";
    case psl_e::digital_input_di52_low_level:
      return "[DI52 (Low level)] (D52L), Digital input DI52 (low level)";
    case psl_e::digital_input_di53_low_level:
      return "[DI53 (Low level)] (D53L), Digital input DI53 (low level)";
    case psl_e::digital_input_di54_low_level:
      return "[DI54 (Low level)] (D54L), Digital input DI54 (low level)";
    case psl_e::digital_input_di55_low_level:
      return "[DI55 (Low level)] (D55L), Digital input DI55 (low level)";
    case psl_e::digital_input_di56_low_level:
      return "[DI56 (Low level)] (D56L), Digital input DI56 (low level)";
    case psl_e::digital_input_di57_low_level:
      return "[DI57 (Low level)] (D57L), Digital input DI57 (low level)";
    case psl_e::digital_input_di58_low_level:
      return "[DI58 (Low level)] (D58L), Digital input DI58 (low level)";
    case psl_e::digital_input_di59_low_level:
      return "[DI59 (Low level)] (D59L), Digital input DI59 (low level)";
    case psl_e::dc_bus_ripple_warning:
      return "[DC Bus Ripple Warn] (DCRW), DC bus ripple warning";
    case psl_e::jockey:
      return "[Jockey] (JOKY), Jockey";
    case psl_e::priming:
      return "[Priming] (PRIM), Priming";
    case psl_e::anti_jam_active:
      return "[Anti-Jam Active] (JAMR), Anti-Jam active";
    case psl_e::pipe_fill:
      return "[Pipe Fill] (FILL), Pipe Fill";
    case psl_e::priming_pump_active:
      return "[Priming Pump Active] (PPON), Priming pump active";
    case psl_e::dry_run_warning:
      return "[Dry Run Warning] (DRYA), Dry run warning";
    case psl_e::pump_low_flow:
      return "[Pump Low Flow ] (PLFA), Pump low flow ";
    case psl_e::process_high_flow_warning:
      return "[Proc High Flow Warn] (HFPA), Process high flow warning";
    case psl_e::inlet_pressure_warning:
      return "[InPress Warning] (IPPA), Inlet pressure warning";
    case psl_e::outlet_pressure_low_warning:
      return "[Low OutPres Warning] (OPLA), Outlet pressure low warning";
    case psl_e::outlet_pressure_high_warning:
      return "[High OutPres Warn] (OPHA), Outlet pressure high warning";
    case psl_e::pump_cycle_warning:
      return "[Pump Cycle Warning] (PCPA), Pump cycle warning";
    case psl_e::anti_jam_warning:
      return "[Anti-Jam Warning] (JAMA), Anti-Jam warning";
    case psl_e::low_flow_warning:
      return "[Low Flow Warning] (LFA), Low flow warning";
    case psl_e::low_pressure_warning:
      return "[Low Pressure Warn] (LPA), Low pressure warning";
    case psl_e::output_pressure_high_switch_warning:
      return "[Switch OutPres Warn] (OPSA), Output pressure high switch warning";
    case psl_e::jockey_pump_active:
      return "[Jockey Pump Active] (JPON), Jockey pump active";
    case psl_e::pump_1_command:
      return "[Pump 1 Cmd] (MPO1), Pump 1 command";
    case psl_e::pump_2_command:
      return "[Pump 2 Cmd] (MPO2), Pump 2 command";
    case psl_e::pump_3_command:
      return "[Pump 3 Cmd] (MPO3), Pump 3 command";
    case psl_e::pump_4_command:
      return "[Pump 4 Cmd] (MPO4), Pump 4 command";
    case psl_e::pump_5_command:
      return "[Pump 5 Cmd] (MPO5), Pump 5 command";
    case psl_e::pump_6_command:
      return "[Pump 6 Cmd] (MPO6), Pump 6 command";
    case psl_e::multi_pump_available_capacity_warning:
      return "[MP Capacity Warn] (MPCA), Multi-Pump available capacity warning";
    case psl_e::lead_pump_not_available:
      return "[Lead Pump Warn] (MPLA), Lead pump not available";
    case psl_e::high_level_warning:
      return "[High Level Warning] (LCHA), High level warning";
    case psl_e::low_level_warning:
      return "[Low Level Warning] (LCLA), Low level warning";
    case psl_e::level_switch_warning:
      return "[Level Switch Warning] (LCWA), Level switch warning";
    case psl_e::multipump_device_warning:
      return "[M/P Device Warn] (MPDA), Multipump device warning";
    case psl_e::multi_pump_master_activated:
      return "[M/P Master Activated] (MPMA), Multi-pump master activated";
    case psl_e::temperature_sensor_ai2_warning:
      return "[Temp Sens AI2 Warn] (TS2A), Temperature sensor AI2 warning";
    case psl_e::temperature_sensor_ai3_warning:
      return "[Temp Sens AI3 Warn] (TS3A), Temperature sensor AI3 warning";
    case psl_e::temperature_sensor_ai4_warning:
      return "[Temp Sens AI4 Warn] (TS4A), Temperature sensor AI4 warning";
    case psl_e::temperature_sensor_ai5_warning:
      return "[Temp Sens AI5 Warn] (TS5A), Temperature sensor AI5 warning";
    case psl_e::customer_warning_5:
      return "[Cust Warning 5] (CAS5), Customer warning 5";
    case psl_e::cabinet_fan_command:
      return "[Cabinet Fan Command] (FCC), Cabinet fan command";
    case psl_e::circuit_breaker_start_pulse:
      return "[CB Start Pulse] (CBEP), Circuit breaker start pulse";
    case psl_e::circuit_breaker_stop_pulse:
      return "[CB Stop Pulse] (CBDP), Circuit breaker stop pulse";
    case psl_e::power_consumption_warning:
      return "[Power Cons Warning] (POWD), Power Consumption warning";
    case psl_e::warning_group_4:
      return "[Warning Grp 4] (AG4), Warning group 4";
    case psl_e::warning_group_5:
      return "[Warning Grp 5] (AG5), Warning group 5";
    case psl_e::fallback_speed:
      return "[Fallback speed] (FRF), Fallback speed";
    case psl_e::speed_maintained:
      return "[Speed Maintained] (RLS), Speed maintained";
    case psl_e::per_type_of_stop:
      return "[Per Type of Stop] (STT), Per type of stop";
    case psl_e::life_cycle_warning_1:
      return "[Life Cycle Warn 1] (LCA1), Life cycle warning 1";
    case psl_e::life_cycle_warning_2:
      return "[Life Cycle Warn 2] (LCA2), Life cycle warning 2";
    case psl_e::ai2_thermal_sensor_warning:
      return "[AI2 Th Warning] (TP2A), AI2 thermal sensor warning";
    case psl_e::ai3_thermal_sensor_warning:
      return "[AI3 Th Warning] (TP3A), AI3 thermal sensor warning";
    case psl_e::ai4_thermal_sensor_warning:
      return "[AI4 Th Warning] (TP4A), AI4 thermal sensor warning";
    case psl_e::ai5_thermal_sensor_warning:
      return "[AI5 Th Warning] (TP5A), AI5 thermal sensor warning";
    case psl_e::ai5_4_20_loss_warning:
      return "[AI5 4-20 Loss Warn] (AP5), AI5 4-20 Loss warning";
    case psl_e::fan_counter_warning:
      return "[Fan Counter Warning] (FCTA), Fan counter warning";
    case psl_e::fan_feedback_warning:
      return "[Fan Feedback Warn] (FFDA), Fan feedback warning";
    case psl_e::power_high_threshold:
      return "[Power High Threshold] (PTHA), Power high threshold";
    case psl_e::power_low_threshold:
      return "[Power Low Threshold] (PTHL), Power low threshold";
    case psl_e::customer_warning_1:
      return "[Cust Warning 1] (CAS1), Customer warning 1";
    case psl_e::customer_warning_2:
      return "[Cust Warning 2] (CAS2), Customer warning 2";
    case psl_e::customer_warning_3:
      return "[Cust Warning 3] (CAS3), Customer warning 3";
    case psl_e::customer_warning_4:
      return "[Cust Warning 4] (CAS4), Customer warning 4";
  }
  return "unknown";
}
constexpr auto format_as(psl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pslin_e enum decleration
enum struct pslin_e : std::uint16_t {
  no = 0,               ///< Not assigned ([Not Assigned] (NO))
  ap1 = 123,            ///< AI1 4-20 loss warning ([AI1 4-20 Loss Warning] (AP1))
  ap2 = 12,             ///< AI2 4-20 Loss warning ([AI2 4-20 Loss Warning] (AP2))
  ap3 = 107,            ///< AI3 4-20 Loss warning ([AI3 4-20 Loss Warning] (AP3))
  ap4 = 108,            ///< AI4 4-20 Loss warning ([AI4 4-20 Loss Warn] (AP4))
  ap5 = 503,            ///< AI5 4-20 Loss warning ([AI5 4-20 Loss Warn] (AP5))
  flt = 1,              ///< Drive in operating state "Fault" ([Operating State Fault] (FLT))
  fr1 = 96,             ///< Reference frequency channel 1 ([Ref Freq Channel 1] (FR1))
  fr2 = 97,             ///< Reference frequency channel 2 ([Ref Freq Channel 2] (FR2))
  fr1b = 100,           ///< Command ch = ch 1B ([ch1B active] (FR1B))
  cd1 = 98,             ///< Command channel 1 ([Cmd Channel 1] (CD1))
  cd2 = 99,             ///< Command channel 1 ([Cmd Channel 2] (CD2))
  yes = 128,            ///< Yes ([Yes] (YES))
  fta = 4,              ///< Motor frequency high threshold reached ([Mot Freq High Thd] (FTA))
  f2a = 13,             ///< Motor frequency high threshold 2 reached ([Mot Freq High Thd 2] (F2A))
  ftal = 18,            ///< Motor frequency low threshold reached ([Mot Freq Low Thd] (FTAL))
  f2al = 19,            ///< Motor frequency low threshold 2 reached ([Mot Freq Low Thd 2] (F2AL))
  li1 = 129,            ///< Digital input 1 ([DI1] (LI1))
  li2 = 130,            ///< Digital input 2 ([DI2] (LI2))
  li3 = 131,            ///< Digital input 3 ([DI3] (LI3))
  li4 = 132,            ///< Digital input 4 ([DI4] (LI4))
  li5 = 133,            ///< Digital input 5 ([DI5] (LI5))
  li6 = 134,            ///< Digital input 6 ([DI6] (LI6))
  li11 = 139,           ///< Digital input 11 ([DI11] (LI11))
  li12 = 140,           ///< Digital input 12 ([DI12] (LI12))
  li13 = 141,           ///< Digital input 13 ([DI13] (LI13))
  li14 = 142,           ///< Digital input 14 ([DI14] (LI14))
  li15 = 143,           ///< Digital input 15 ([DI15] (LI15))
  li16 = 144,           ///< Digital input 16 ([DI16] (LI16))
  low_level_1 = 272,    ///< Digital input DI1 (low level) ([DI1 (Low level)] (L1L))
  low_level_2 = 273,    ///< Digital input DI2 (low level) ([DI2 (Low level)] (L2L))
  low_level_3 = 274,    ///< Digital input DI3 (low level) ([DI3 (Low level)] (L3L))
  low_level_4 = 275,    ///< Digital input DI4 (low level) ([DI4 (Low level)] (L4L))
  low_level_5 = 276,    ///< Digital input DI5 (low level) ([DI5 (Low level)] (L5L))
  low_level_6 = 277,    ///< Digital input DI6 (low level) ([DI6 (Low level)] (L6L))
  low_level_11 = 282,   ///< Digital input DI1 (low level) ([DI11 (Low level)] (L11L))
  low_level_12 = 283,   ///< Digital input DI12 (low level) ([DI12 (Low level)] (L12L))
  low_level_13 = 284,   ///< Digital input DI13 (low level) ([DI13 (Low level)] (L13L))
  low_level_14 = 285,   ///< Digital input DI14 (low level) ([DI14 (Low level)] (L14L))
  low_level_15 = 286,   ///< Digital input DI15 (low level) ([DI15 (Low level)] (L15L))
  low_level_16 = 287,   ///< Digital input DI16 (low level) ([DI16 (Low level)] (L16L))
  cd00 = 160,           ///< Bit 0 digital input ctrl word ([CD00] (CD00))
  cd01 = 161,           ///< Bit 1 digital input ctrl word ([CD01] (CD01))
  cd02 = 162,           ///< Bit 2 digital input ctrl word ([CD02] (CD02))
  cd03 = 163,           ///< Bit 3 digital input ctrl word ([CD03] (CD03))
  cd04 = 164,           ///< Bit 4 digital input ctrl word ([CD04] (CD04))
  cd05 = 165,           ///< Bit 5 digital input ctrl word ([CD05] (CD05))
  cd06 = 166,           ///< Bit 6 digital input ctrl word ([CD06] (CD06))
  cd07 = 167,           ///< Bit 7 digital input ctrl word ([CD07] (CD07))
  cd08 = 168,           ///< Bit 8 digital input ctrl word ([CD08] (CD08))
  cd09 = 169,           ///< Bit 9 digital input ctrl word ([CD09] (CD09))
  cd10 = 170,           ///< Bit10 digital input ctrl word ([CD10] (CD10))
  cd11 = 171,           ///< Bit11 digital input ctrl word ([CD11] (CD11))
  cd12 = 172,           ///< Bit12 digital input ctrl word ([CD12] (CD12))
  cd13 = 173,           ///< Bit13 digital input ctrl word ([CD13] (CD13))
  cd14 = 174,           ///< Bit14 digital input ctrl word ([CD14] (CD14))
  cd15 = 175,           ///< Bit15 digital input ctrl word ([CD15] (CD15))
  c101 = 177,           ///< Bit 1 Modbus ctrl word ([C101] (C101))
  c102 = 178,           ///< Bit 2 Modbus ctrl word ([C102] (C102))
  c103 = 179,           ///< Bit 3 Modbus ctrl word ([C103] (C103))
  c104 = 180,           ///< Bit 4 Modbus ctrl word ([C104] (C104))
  c105 = 181,           ///< Bit 5 Modbus ctrl word ([C105] (C105))
  c106 = 182,           ///< Bit 6 Modbus ctrl word ([C106] (C106))
  c107 = 183,           ///< Bit 7 Modbus ctrl word ([C107] (C107))
  c108 = 184,           ///< Bit 8 Modbus ctrl word ([C108] (C108))
  c109 = 185,           ///< Bit 9 Modbus ctrl word ([C109] (C109))
  c110 = 186,           ///< Bit 10 Modbus ctrl word ([C110] (C110))
  c111 = 187,           ///< Bit 11 Modbus ctrl word ([C111] (C111))
  c112 = 188,           ///< Bit 12 Modbus ctrl word ([C112] (C112))
  c113 = 189,           ///< Bit 13 Modbus ctrl word ([C113] (C113))
  c114 = 190,           ///< Bit 14 Modbus ctrl word ([C114] (C114))
  c115 = 191,           ///< Bit 15 Modbus ctrl word ([C115] (C115))
  c201 = 193,           ///< Bit 1 CANopen ctrl word ([C201] (C201))
  c202 = 194,           ///< Bit 2 CANopen ctrl word ([C202] (C202))
  c203 = 195,           ///< Bit 3 CANopen ctrl word ([C203] (C203))
  c204 = 196,           ///< Bit 4 CANopen ctrl word ([C204] (C204))
  c205 = 197,           ///< Bit 5 CANopen ctrl word ([C205] (C205))
  c206 = 198,           ///< Bit 6 CANopen ctrl word ([C206] (C206))
  c207 = 199,           ///< Bit 7 CANopen ctrl word ([C207] (C207))
  c208 = 200,           ///< Bit 8 CANopen ctrl word ([C208] (C208))
  c209 = 201,           ///< Bit 9 CANopen ctrl word ([C209] (C209))
  c210 = 202,           ///< Bit 10 CANopen ctrl word ([C210] (C210))
  c211 = 203,           ///< Bit 11 CANopen ctrl word ([C211] (C211))
  c212 = 204,           ///< Bit 12 CANopen ctrl word ([C212] (C212))
  c213 = 205,           ///< Bit 13 CANopen ctrl word ([C213] (C213))
  c214 = 206,           ///< Bit 14 CANopen ctrl word ([C214] (C214))
  c215 = 207,           ///< Bit 15 CANopen ctrl word ([C215] (C215))
  c301 = 209,           ///< Bit 1 Com module ctrl word ([C301] (C301))
  c302 = 210,           ///< Bit 2 Com module ctrl word ([C302] (C302))
  c303 = 211,           ///< Bit 3 Com module ctrl word ([C303] (C303))
  c304 = 212,           ///< Bit 4 Com module ctrl word ([C304] (C304))
  c305 = 213,           ///< Bit 5 Com module ctrl word ([C305] (C305))
  c306 = 214,           ///< Bit 6 Com module ctrl word ([C306] (C306))
  c307 = 215,           ///< Bit 7 Com module ctrl word ([C307] (C307))
  c308 = 216,           ///< Bit 8 Com module ctrl word ([C308] (C308))
  c309 = 217,           ///< Bit 9 Com module ctrl word ([C309] (C309))
  c310 = 218,           ///< Bit 10 Com module ctrl word ([C310] (C310))
  c311 = 219,           ///< Bit 11 Com module ctrl word ([C311] (C311))
  c312 = 220,           ///< Bit 12 Com module ctrl word ([C312] (C312))
  c313 = 221,           ///< Bit 13 Com module ctrl word ([C313] (C313))
  c314 = 222,           ///< Bit 14 Com module ctrl word ([C314] (C314))
  c315 = 223,           ///< Bit 15 Com module ctrl word ([C315] (C315))
  c501 = 241,           ///< C501 ([C501] (C501))
  c502 = 242,           ///< C502 ([C502] (C502))
  c503 = 243,           ///< C503 ([C503] (C503))
  c504 = 244,           ///< C504 ([C504] (C504))
  c505 = 245,           ///< C505 ([C505] (C505))
  c506 = 246,           ///< C506 ([C506] (C506))
  c507 = 247,           ///< C507 ([C507] (C507))
  c508 = 248,           ///< C508 ([C508] (C508))
  c509 = 249,           ///< C509 ([C509] (C509))
  c510 = 250,           ///< C510 ([C510] (C510))
  c511 = 251,           ///< C511 ([C511] (C511))
  c512 = 252,           ///< C512 ([C512] (C512))
  c513 = 253,           ///< C513 ([C513] (C513))
  c514 = 254,           ///< C514 ([C514] (C514))
  c515 = 255,           ///< C515 ([C515] (C515))
  high_level_50 = 302,  ///< Digital input DI50 (High level) ([DI50 (High Level)] (D50H))
  high_level_51 = 303,  ///< Digital input DI51 (High level) ([DI51 (High Level)] (D51H))
  high_level_52 = 304,  ///< Digital input DI52 (High level) ([DI52 (High Level)] (D52H))
  high_level_53 = 305,  ///< Digital input DI53 (High level) ([DI53 (High Level)] (D53H))
  high_level_54 = 306,  ///< Digital input DI54 (High level) ([DI54 (High Level)] (D54H))
  high_level_55 = 307,  ///< Digital input DI55 (High level) ([DI55 (High Level)] (D55H))
  high_level_56 = 308,  ///< Digital input DI56 (High level) ([DI56 (High Level)] (D56H))
  high_level_57 = 309,  ///< Digital input DI57 (High level) ([DI57 (High Level)] (D57H))
  high_level_58 = 310,  ///< Digital input DI58 (High level) ([DI58 (High Level)] (D58H))
  high_level_59 = 311,  ///< Digital input DI59 (High level) ([DI59 (High Level)] (D59H))
  low_level_50 = 312,   ///< Digital input DI50 (low level) ([DI50 (Low level)] (D50L))
  low_level_51 = 313,   ///< Digital input DI51 (low level) ([DI51 (Low level)] (D51L))
  low_level_52 = 314,   ///< Digital input DI52 (low level) ([DI52 (Low level)] (D52L))
  low_level_53 = 315,   ///< Digital input DI53 (low level) ([DI53 (Low level)] (D53L))
  low_level_54 = 316,   ///< Digital input DI54 (low level) ([DI54 (Low level)] (D54L))
  low_level_55 = 317,   ///< Digital input DI55 (low level) ([DI55 (Low level)] (D55L))
  low_level_56 = 318,   ///< Digital input DI56 (low level) ([DI56 (Low level)] (D56L))
  low_level_57 = 319,   ///< Digital input DI57 (low level) ([DI57 (Low level)] (D57L))
  low_level_58 = 320,   ///< Digital input DI58 (low level) ([DI58 (Low level)] (D58L))
  low_level_59 = 321,   ///< Digital input DI59 (low level) ([DI59 (Low level)] (D59L))
};
[[nodiscard]] constexpr auto enum_desc(pslin_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pslin_e::no:
      return "[Not Assigned] (NO), Not assigned";
    case pslin_e::ap1:
      return "[AI1 4-20 Loss Warning] (AP1), AI1 4-20 loss warning";
    case pslin_e::ap2:
      return "[AI2 4-20 Loss Warning] (AP2), AI2 4-20 Loss warning";
    case pslin_e::ap3:
      return "[AI3 4-20 Loss Warning] (AP3), AI3 4-20 Loss warning";
    case pslin_e::ap4:
      return "[AI4 4-20 Loss Warn] (AP4), AI4 4-20 Loss warning";
    case pslin_e::ap5:
      return "[AI5 4-20 Loss Warn] (AP5), AI5 4-20 Loss warning";
    case pslin_e::flt:
      return "[Operating State Fault] (FLT), Drive in operating state Fault";
    case pslin_e::fr1:
      return "[Ref Freq Channel 1] (FR1), Reference frequency channel 1";
    case pslin_e::fr2:
      return "[Ref Freq Channel 2] (FR2), Reference frequency channel 2";
    case pslin_e::fr1b:
      return "[ch1B active] (FR1B), Command ch = ch 1B";
    case pslin_e::cd1:
      return "[Cmd Channel 1] (CD1), Command channel 1";
    case pslin_e::cd2:
      return "[Cmd Channel 2] (CD2), Command channel 1";
    case pslin_e::yes:
      return "[Yes] (YES), Yes";
    case pslin_e::fta:
      return "[Mot Freq High Thd] (FTA), Motor frequency high threshold reached";
    case pslin_e::f2a:
      return "[Mot Freq High Thd 2] (F2A), Motor frequency high threshold 2 reached";
    case pslin_e::ftal:
      return "[Mot Freq Low Thd] (FTAL), Motor frequency low threshold reached";
    case pslin_e::f2al:
      return "[Mot Freq Low Thd 2] (F2AL), Motor frequency low threshold 2 reached";
    case pslin_e::li1:
      return "[DI1] (LI1), Digital input 1";
    case pslin_e::li2:
      return "[DI2] (LI2), Digital input 2";
    case pslin_e::li3:
      return "[DI3] (LI3), Digital input 3";
    case pslin_e::li4:
      return "[DI4] (LI4), Digital input 4";
    case pslin_e::li5:
      return "[DI5] (LI5), Digital input 5";
    case pslin_e::li6:
      return "[DI6] (LI6), Digital input 6";
    case pslin_e::li11:
      return "[DI11] (LI11), Digital input 11";
    case pslin_e::li12:
      return "[DI12] (LI12), Digital input 12";
    case pslin_e::li13:
      return "[DI13] (LI13), Digital input 13";
    case pslin_e::li14:
      return "[DI14] (LI14), Digital input 14";
    case pslin_e::li15:
      return "[DI15] (LI15), Digital input 15";
    case pslin_e::li16:
      return "[DI16] (LI16), Digital input 16";
    case pslin_e::low_level_1:
      return "[DI1 (Low level)] (L1L), Digital input DI1 (low level)";
    case pslin_e::low_level_2:
      return "[DI2 (Low level)] (L2L), Digital input DI2 (low level)";
    case pslin_e::low_level_3:
      return "[DI3 (Low level)] (L3L), Digital input DI3 (low level)";
    case pslin_e::low_level_4:
      return "[DI4 (Low level)] (L4L), Digital input DI4 (low level)";
    case pslin_e::low_level_5:
      return "[DI5 (Low level)] (L5L), Digital input DI5 (low level)";
    case pslin_e::low_level_6:
      return "[DI6 (Low level)] (L6L), Digital input DI6 (low level)";
    case pslin_e::low_level_11:
      return "[DI11 (Low level)] (L11L), Digital input DI1 (low level)";
    case pslin_e::low_level_12:
      return "[DI12 (Low level)] (L12L), Digital input DI12 (low level)";
    case pslin_e::low_level_13:
      return "[DI13 (Low level)] (L13L), Digital input DI13 (low level)";
    case pslin_e::low_level_14:
      return "[DI14 (Low level)] (L14L), Digital input DI14 (low level)";
    case pslin_e::low_level_15:
      return "[DI15 (Low level)] (L15L), Digital input DI15 (low level)";
    case pslin_e::low_level_16:
      return "[DI16 (Low level)] (L16L), Digital input DI16 (low level)";
    case pslin_e::cd00:
      return "[CD00] (CD00), Bit 0 digital input ctrl word";
    case pslin_e::cd01:
      return "[CD01] (CD01), Bit 1 digital input ctrl word";
    case pslin_e::cd02:
      return "[CD02] (CD02), Bit 2 digital input ctrl word";
    case pslin_e::cd03:
      return "[CD03] (CD03), Bit 3 digital input ctrl word";
    case pslin_e::cd04:
      return "[CD04] (CD04), Bit 4 digital input ctrl word";
    case pslin_e::cd05:
      return "[CD05] (CD05), Bit 5 digital input ctrl word";
    case pslin_e::cd06:
      return "[CD06] (CD06), Bit 6 digital input ctrl word";
    case pslin_e::cd07:
      return "[CD07] (CD07), Bit 7 digital input ctrl word";
    case pslin_e::cd08:
      return "[CD08] (CD08), Bit 8 digital input ctrl word";
    case pslin_e::cd09:
      return "[CD09] (CD09), Bit 9 digital input ctrl word";
    case pslin_e::cd10:
      return "[CD10] (CD10), Bit10 digital input ctrl word";
    case pslin_e::cd11:
      return "[CD11] (CD11), Bit11 digital input ctrl word";
    case pslin_e::cd12:
      return "[CD12] (CD12), Bit12 digital input ctrl word";
    case pslin_e::cd13:
      return "[CD13] (CD13), Bit13 digital input ctrl word";
    case pslin_e::cd14:
      return "[CD14] (CD14), Bit14 digital input ctrl word";
    case pslin_e::cd15:
      return "[CD15] (CD15), Bit15 digital input ctrl word";
    case pslin_e::c101:
      return "[C101] (C101), Bit 1 Modbus ctrl word";
    case pslin_e::c102:
      return "[C102] (C102), Bit 2 Modbus ctrl word";
    case pslin_e::c103:
      return "[C103] (C103), Bit 3 Modbus ctrl word";
    case pslin_e::c104:
      return "[C104] (C104), Bit 4 Modbus ctrl word";
    case pslin_e::c105:
      return "[C105] (C105), Bit 5 Modbus ctrl word";
    case pslin_e::c106:
      return "[C106] (C106), Bit 6 Modbus ctrl word";
    case pslin_e::c107:
      return "[C107] (C107), Bit 7 Modbus ctrl word";
    case pslin_e::c108:
      return "[C108] (C108), Bit 8 Modbus ctrl word";
    case pslin_e::c109:
      return "[C109] (C109), Bit 9 Modbus ctrl word";
    case pslin_e::c110:
      return "[C110] (C110), Bit 10 Modbus ctrl word";
    case pslin_e::c111:
      return "[C111] (C111), Bit 11 Modbus ctrl word";
    case pslin_e::c112:
      return "[C112] (C112), Bit 12 Modbus ctrl word";
    case pslin_e::c113:
      return "[C113] (C113), Bit 13 Modbus ctrl word";
    case pslin_e::c114:
      return "[C114] (C114), Bit 14 Modbus ctrl word";
    case pslin_e::c115:
      return "[C115] (C115), Bit 15 Modbus ctrl word";
    case pslin_e::c201:
      return "[C201] (C201), Bit 1 CANopen ctrl word";
    case pslin_e::c202:
      return "[C202] (C202), Bit 2 CANopen ctrl word";
    case pslin_e::c203:
      return "[C203] (C203), Bit 3 CANopen ctrl word";
    case pslin_e::c204:
      return "[C204] (C204), Bit 4 CANopen ctrl word";
    case pslin_e::c205:
      return "[C205] (C205), Bit 5 CANopen ctrl word";
    case pslin_e::c206:
      return "[C206] (C206), Bit 6 CANopen ctrl word";
    case pslin_e::c207:
      return "[C207] (C207), Bit 7 CANopen ctrl word";
    case pslin_e::c208:
      return "[C208] (C208), Bit 8 CANopen ctrl word";
    case pslin_e::c209:
      return "[C209] (C209), Bit 9 CANopen ctrl word";
    case pslin_e::c210:
      return "[C210] (C210), Bit 10 CANopen ctrl word";
    case pslin_e::c211:
      return "[C211] (C211), Bit 11 CANopen ctrl word";
    case pslin_e::c212:
      return "[C212] (C212), Bit 12 CANopen ctrl word";
    case pslin_e::c213:
      return "[C213] (C213), Bit 13 CANopen ctrl word";
    case pslin_e::c214:
      return "[C214] (C214), Bit 14 CANopen ctrl word";
    case pslin_e::c215:
      return "[C215] (C215), Bit 15 CANopen ctrl word";
    case pslin_e::c301:
      return "[C301] (C301), Bit 1 Com module ctrl word";
    case pslin_e::c302:
      return "[C302] (C302), Bit 2 Com module ctrl word";
    case pslin_e::c303:
      return "[C303] (C303), Bit 3 Com module ctrl word";
    case pslin_e::c304:
      return "[C304] (C304), Bit 4 Com module ctrl word";
    case pslin_e::c305:
      return "[C305] (C305), Bit 5 Com module ctrl word";
    case pslin_e::c306:
      return "[C306] (C306), Bit 6 Com module ctrl word";
    case pslin_e::c307:
      return "[C307] (C307), Bit 7 Com module ctrl word";
    case pslin_e::c308:
      return "[C308] (C308), Bit 8 Com module ctrl word";
    case pslin_e::c309:
      return "[C309] (C309), Bit 9 Com module ctrl word";
    case pslin_e::c310:
      return "[C310] (C310), Bit 10 Com module ctrl word";
    case pslin_e::c311:
      return "[C311] (C311), Bit 11 Com module ctrl word";
    case pslin_e::c312:
      return "[C312] (C312), Bit 12 Com module ctrl word";
    case pslin_e::c313:
      return "[C313] (C313), Bit 13 Com module ctrl word";
    case pslin_e::c314:
      return "[C314] (C314), Bit 14 Com module ctrl word";
    case pslin_e::c315:
      return "[C315] (C315), Bit 15 Com module ctrl word";
    case pslin_e::c501:
      return "[C501] (C501), C501";
    case pslin_e::c502:
      return "[C502] (C502), C502";
    case pslin_e::c503:
      return "[C503] (C503), C503";
    case pslin_e::c504:
      return "[C504] (C504), C504";
    case pslin_e::c505:
      return "[C505] (C505), C505";
    case pslin_e::c506:
      return "[C506] (C506), C506";
    case pslin_e::c507:
      return "[C507] (C507), C507";
    case pslin_e::c508:
      return "[C508] (C508), C508";
    case pslin_e::c509:
      return "[C509] (C509), C509";
    case pslin_e::c510:
      return "[C510] (C510), C510";
    case pslin_e::c511:
      return "[C511] (C511), C511";
    case pslin_e::c512:
      return "[C512] (C512), C512";
    case pslin_e::c513:
      return "[C513] (C513), C513";
    case pslin_e::c514:
      return "[C514] (C514), C514";
    case pslin_e::c515:
      return "[C515] (C515), C515";
    case pslin_e::high_level_50:
      return "[DI50 (High Level)] (D50H), Digital input DI50 (High level)";
    case pslin_e::high_level_51:
      return "[DI51 (High Level)] (D51H), Digital input DI51 (High level)";
    case pslin_e::high_level_52:
      return "[DI52 (High Level)] (D52H), Digital input DI52 (High level)";
    case pslin_e::high_level_53:
      return "[DI53 (High Level)] (D53H), Digital input DI53 (High level)";
    case pslin_e::high_level_54:
      return "[DI54 (High Level)] (D54H), Digital input DI54 (High level)";
    case pslin_e::high_level_55:
      return "[DI55 (High Level)] (D55H), Digital input DI55 (High level)";
    case pslin_e::high_level_56:
      return "[DI56 (High Level)] (D56H), Digital input DI56 (High level)";
    case pslin_e::high_level_57:
      return "[DI57 (High Level)] (D57H), Digital input DI57 (High level)";
    case pslin_e::high_level_58:
      return "[DI58 (High Level)] (D58H), Digital input DI58 (High level)";
    case pslin_e::high_level_59:
      return "[DI59 (High Level)] (D59H), Digital input DI59 (High level)";
    case pslin_e::low_level_50:
      return "[DI50 (Low level)] (D50L), Digital input DI50 (low level)";
    case pslin_e::low_level_51:
      return "[DI51 (Low level)] (D51L), Digital input DI51 (low level)";
    case pslin_e::low_level_52:
      return "[DI52 (Low level)] (D52L), Digital input DI52 (low level)";
    case pslin_e::low_level_53:
      return "[DI53 (Low level)] (D53L), Digital input DI53 (low level)";
    case pslin_e::low_level_54:
      return "[DI54 (Low level)] (D54L), Digital input DI54 (low level)";
    case pslin_e::low_level_55:
      return "[DI55 (Low level)] (D55L), Digital input DI55 (low level)";
    case pslin_e::low_level_56:
      return "[DI56 (Low level)] (D56L), Digital input DI56 (low level)";
    case pslin_e::low_level_57:
      return "[DI57 (Low level)] (D57L), Digital input DI57 (low level)";
    case pslin_e::low_level_58:
      return "[DI58 (Low level)] (D58L), Digital input DI58 (low level)";
    case pslin_e::low_level_59:
      return "[DI59 (Low level)] (D59L), Digital input DI59 (low level)";
  }
  return "unknown";
}
constexpr auto format_as(pslin_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pss_e enum decleration
enum struct pss_e : std::uint16_t {
  nact = 0,  ///< Pre-settings not locked ([Not locked] (NACT))
  act = 1,   ///< Pre-settings locked ([Locked] (ACT))
};
[[nodiscard]] constexpr auto enum_desc(pss_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pss_e::nact:
      return "[Not locked] (NACT), Pre-settings not locked";
    case pss_e::act:
      return "[Locked] (ACT), Pre-settings locked";
  }
  return "unknown";
}
constexpr auto format_as(pss_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of psst_e enum decleration
enum struct psst_e : std::uint16_t {
  no = 0,    ///< No password defined ([No password defined] (NO))
  ulk = 1,   ///< Password is unlocked ([Password is unlocked] (ULK))
  lock = 2,  ///< Password is locked ([Password is locked] (LOCK))
};
[[nodiscard]] constexpr auto enum_desc(psst_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case psst_e::no:
      return "[No password defined] (NO), No password defined";
    case psst_e::ulk:
      return "[Password is unlocked] (ULK), Password is unlocked";
    case psst_e::lock:
      return "[Password is locked] (LOCK), Password is locked";
  }
  return "unknown";
}
constexpr auto format_as(psst_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pst_e enum decleration
enum struct pst_e : std::uint16_t {
  no = 0,   ///< Stop key no priority ([Stop Key No Priority] (NO))
  yes = 1,  ///< Stop key priority ([Stop Key Priority] (YES))
  all = 2,  ///< Stop key priority all CMD channels ([Stop Key Priority All ] (ALL))
};
[[nodiscard]] constexpr auto enum_desc(pst_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pst_e::no:
      return "[Stop Key No Priority] (NO), Stop key no priority";
    case pst_e::yes:
      return "[Stop Key Priority] (YES), Stop key priority";
    case pst_e::all:
      return "[Stop Key Priority All ] (ALL), Stop key priority all CMD channels";
  }
  return "unknown";
}
constexpr auto format_as(pst_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pvis_e enum decleration
enum struct pvis_e : std::uint16_t {
  act = 0,  ///< Active parameters ([Active] (ACT))
  all = 1,  ///< All parameters ([All] (ALL))
};
[[nodiscard]] constexpr auto enum_desc(pvis_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pvis_e::act:
      return "[Active] (ACT), Active parameters";
    case pvis_e::all:
      return "[All] (ALL), All parameters";
  }
  return "unknown";
}
constexpr auto format_as(pvis_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pxct_e enum decleration
enum struct pxct_e : std::uint16_t {
  none = 0,  ///< Undefined ([Undefined] (NONE))
  lead = 1,  ///< Lead pump ([Lead] (LEAD))
  laf = 2,   ///< Lead or auxiliary fixed speed pump ([Lead or Auxiliary] (LAF))
  lav = 3,   ///< Lead or auxiliary variable speed pump ([Lead or Aux. Variable] (LAV))
  auxf = 4,  ///< Auxiliary fixed speed pump ([Auxiliary] (AUXF))
  auxv = 5,  ///< Auxiliary variable speed pump ([Auxiliary Variable] (AUXV))
  err = 6,   ///< Error ([Error] (ERR))
};
[[nodiscard]] constexpr auto enum_desc(pxct_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pxct_e::none:
      return "[Undefined] (NONE), Undefined";
    case pxct_e::lead:
      return "[Lead] (LEAD), Lead pump";
    case pxct_e::laf:
      return "[Lead or Auxiliary] (LAF), Lead or auxiliary fixed speed pump";
    case pxct_e::lav:
      return "[Lead or Aux. Variable] (LAV), Lead or auxiliary variable speed pump";
    case pxct_e::auxf:
      return "[Auxiliary] (AUXF), Auxiliary fixed speed pump";
    case pxct_e::auxv:
      return "[Auxiliary Variable] (AUXV), Auxiliary variable speed pump";
    case pxct_e::err:
      return "[Error] (ERR), Error";
  }
  return "unknown";
}
constexpr auto format_as(pxct_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of pxs_e enum decleration
enum struct pxs_e : std::uint16_t {
  none = 0,  ///< Not configured ([None] (NONE))
  navl = 1,  ///< Not Available ([Not Available] (NAVL))
  rdy = 2,   ///< Ready ([Ready] (RDY))
  run = 3,   ///< Running ([Running] (RUN))
};
[[nodiscard]] constexpr auto enum_desc(pxs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case pxs_e::none:
      return "[None] (NONE), Not configured";
    case pxs_e::navl:
      return "[Not Available] (NAVL), Not Available";
    case pxs_e::rdy:
      return "[Ready] (RDY), Ready";
    case pxs_e::run:
      return "[Running] (RUN), Running";
  }
  return "unknown";
}
constexpr auto format_as(pxs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of qstd_e enum decleration
enum struct qstd_e : std::uint16_t {
  fst2 = 2,  ///< Fast stop then disable voltage ([Fast stop then disable voltage] (FST2))
  fst6 = 6,  ///< Fast stop then stay in quick stop state ([Fast stop then stay in quick stop state] (FST6))
};
[[nodiscard]] constexpr auto enum_desc(qstd_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case qstd_e::fst2:
      return "[Fast stop then disable voltage] (FST2), Fast stop then disable voltage";
    case qstd_e::fst6:
      return "[Fast stop then stay in quick stop state] (FST6), Fast stop then stay in quick stop state";
  }
  return "unknown";
}
constexpr auto format_as(qstd_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of rds_e enum decleration
enum struct rds_e : std::uint16_t {
  automatic = 0,  ///< Auto detected ([Auto] (AUTO))
  full_10M = 1,   ///< 10Mbps full duplex ([10M. full] (10F))
  half_10M = 2,   ///< 10Mbps half duplex ([10M. half] (10H))
  full_100M = 3,  ///< 100Mbps full duplex ([100M. full] (100F))
  half_100M = 4,  ///< 100Mbps half duplex ([100M. half] (100H))
};
[[nodiscard]] constexpr auto enum_desc(rds_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case rds_e::automatic:
      return "[Auto] (AUTO), Auto detected";
    case rds_e::full_10M:
      return "[10M. full] (10F), 10Mbps full duplex";
    case rds_e::half_10M:
      return "[10M. half] (10H), 10Mbps half duplex";
    case rds_e::full_100M:
      return "[100M. full] (100F), 100Mbps full duplex";
    case rds_e::half_100M:
      return "[100M. half] (100H), 100Mbps half duplex";
  }
  return "unknown";
}
constexpr auto format_as(rds_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of rpr_e enum decleration
enum struct rpr_e : std::uint16_t {
  no = 0,     ///< No ([No] (NO))
  rth = 2,    ///< Run time reset ([Run Time Reset] (RTH))
  rthi = 3,   ///< Internal runtime reset ([Internal Run Time Reset] (RTHI))
  pth = 4,    ///< Power ON time reset ([Power ON Time Reset] (PTH))
  fth = 7,    ///< Reset fan counter ([Reset Fan Counter] (FTH))
  pthi = 8,   ///< Internal power ON time Reset ([In Power ON Time Reset] (PTHI))
  gthi = 9,   ///< Clear GTHI ([Clear GTHI] (GTHI))
  lthi = 10,  ///< Clear LTHI ([Clear LTHI] (LTHI))
  nsm = 11,   ///< Clear NSM ([Clear NSM] (NSM))
  nsmi = 12,  ///< Clear NSMI ([Clear NSMI] (NSMI))
  fbat = 13,  ///< Clear AFE fan operation time ([Clear AFE Fan] (FBAT))
  fct = 14,   ///< Clear cabinet fan operation time ([Clear Cabinet Fan] (FCT))
  efyk = 20,  ///< Efficiency MAX ([Efficiency MAX] (EFYK))
  efyj = 21,  ///< Efficiency MIN ([Efficiency MIN] (EFYJ))
  fs1k = 22,  ///< Flow rate MAX ([Flow Rate MAX] (FS1K))
  fs1j = 23,  ///< Flow rate MIN ([Flow Rate MIN] (FS1J))
  fs1c = 24,  ///< Reset total quantity ([Reset Total Quantity] (FS1C))
  brth = 30,  ///< Clear BRTH ([Clear BRTH] (BRTH))
  brti = 31,  ///< Clear BRTI ([Clear BRTI] (BRTI))
  bpth = 32,  ///< Clear AFE power ON time ([Clear AFE Power ON Time] (BPTH))
  bpti = 33,  ///< Clear BPTI ([Clear BPTI] (BPTI))
  bnsa = 34,  ///< Clear AFE brick number of start ([Clear AFE Nb. start] (BNSA))
  bnsi = 35,  ///< Clear BNSI ([Clear BNSI] (BNSI))
  bgth = 36,  ///< Clear AFE regen time  ([Clear AFE Regen Time ] (BGTH))
  bgti = 37,  ///< Clear BGTI ([Clear BGTI] (BGTI))
  blmi = 38,  ///< Clear BLMI ([Clear BLMI] (BLMI))
  blgi = 39,  ///< Clear BLGI ([Clear BLGI] (BLGI))
  all = 64,   ///< Reset all counters ([Reset all ] (ALL))
};
[[nodiscard]] constexpr auto enum_desc(rpr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case rpr_e::no:
      return "[No] (NO), No";
    case rpr_e::rth:
      return "[Run Time Reset] (RTH), Run time reset";
    case rpr_e::rthi:
      return "[Internal Run Time Reset] (RTHI), Internal runtime reset";
    case rpr_e::pth:
      return "[Power ON Time Reset] (PTH), Power ON time reset";
    case rpr_e::fth:
      return "[Reset Fan Counter] (FTH), Reset fan counter";
    case rpr_e::pthi:
      return "[In Power ON Time Reset] (PTHI), Internal power ON time Reset";
    case rpr_e::gthi:
      return "[Clear GTHI] (GTHI), Clear GTHI";
    case rpr_e::lthi:
      return "[Clear LTHI] (LTHI), Clear LTHI";
    case rpr_e::nsm:
      return "[Clear NSM] (NSM), Clear NSM";
    case rpr_e::nsmi:
      return "[Clear NSMI] (NSMI), Clear NSMI";
    case rpr_e::fbat:
      return "[Clear AFE Fan] (FBAT), Clear AFE fan operation time";
    case rpr_e::fct:
      return "[Clear Cabinet Fan] (FCT), Clear cabinet fan operation time";
    case rpr_e::efyk:
      return "[Efficiency MAX] (EFYK), Efficiency MAX";
    case rpr_e::efyj:
      return "[Efficiency MIN] (EFYJ), Efficiency MIN";
    case rpr_e::fs1k:
      return "[Flow Rate MAX] (FS1K), Flow rate MAX";
    case rpr_e::fs1j:
      return "[Flow Rate MIN] (FS1J), Flow rate MIN";
    case rpr_e::fs1c:
      return "[Reset Total Quantity] (FS1C), Reset total quantity";
    case rpr_e::brth:
      return "[Clear BRTH] (BRTH), Clear BRTH";
    case rpr_e::brti:
      return "[Clear BRTI] (BRTI), Clear BRTI";
    case rpr_e::bpth:
      return "[Clear AFE Power ON Time] (BPTH), Clear AFE power ON time";
    case rpr_e::bpti:
      return "[Clear BPTI] (BPTI), Clear BPTI";
    case rpr_e::bnsa:
      return "[Clear AFE Nb. start] (BNSA), Clear AFE brick number of start";
    case rpr_e::bnsi:
      return "[Clear BNSI] (BNSI), Clear BNSI";
    case rpr_e::bgth:
      return "[Clear AFE Regen Time ] (BGTH), Clear AFE regen time ";
    case rpr_e::bgti:
      return "[Clear BGTI] (BGTI), Clear BGTI";
    case rpr_e::blmi:
      return "[Clear BLMI] (BLMI), Clear BLMI";
    case rpr_e::blgi:
      return "[Clear BLGI] (BLGI), Clear BLGI";
    case rpr_e::all:
      return "[Reset all ] (ALL), Reset all counters";
  }
  return "unknown";
}
constexpr auto format_as(rpr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of rpt_e enum decleration
enum struct rpt_e : std::uint16_t {
  lin = 0,  ///< Linear ramp ([Linear] (LIN))
  s = 1,    ///< S-Ramp ([S-Ramp] (S))
  u = 2,    ///< U-Ramp ([U-Ramp] (U))
  cus = 3,  ///< Ramp customized ([Customized] (CUS))
};
[[nodiscard]] constexpr auto enum_desc(rpt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case rpt_e::lin:
      return "[Linear] (LIN), Linear ramp";
    case rpt_e::s:
      return "[S-Ramp] (S), S-Ramp";
    case rpt_e::u:
      return "[U-Ramp] (U), U-Ramp";
    case rpt_e::cus:
      return "[Customized] (CUS), Ramp customized";
  }
  return "unknown";
}
constexpr auto format_as(rpt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of scs_e enum decleration
enum struct scs_e : std::uint16_t {
  no = 0,    ///< No ([No] (NO))
  str0 = 1,  ///< Save configuration 0 ([Config 0] (STR0))
  str1 = 2,  ///< Save configuration 1 ([Config 1] (STR1))
  str2 = 3,  ///< Save configuration 2 ([Config 2] (STR2))
  str3 = 4,  ///< Config 3 ([Config 3] (STR3))
};
[[nodiscard]] constexpr auto enum_desc(scs_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case scs_e::no:
      return "[No] (NO), No";
    case scs_e::str0:
      return "[Config 0] (STR0), Save configuration 0";
    case scs_e::str1:
      return "[Config 1] (STR1), Save configuration 1";
    case scs_e::str2:
      return "[Config 2] (STR2), Save configuration 2";
    case scs_e::str3:
      return "[Config 3] (STR3), Config 3";
  }
  return "unknown";
}
constexpr auto format_as(scs_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of sfdr_e enum decleration
enum struct sfdr_e : std::uint16_t {
  init = 0,  ///< Initialization ([Initialization] (INIT))
  idle = 1,  ///< Not active ([Not Active] (IDLE))
  ope = 2,   ///< Operational ([Operational] (OPE))
  rdy = 4,   ///< Ready ([Ready] (RDY))
  ipc = 5,   ///< IP configuration ([IP Configuration] (IPC))
  uncf = 7,  ///< Not configured ([Not Configured] (UNCF))
  get = 8,   ///< Reading configuration ([Reading Configuration] (GET))
  set = 9,   ///< Writing configuration ([Writing Configuration] (SET))
  app = 10,  ///< Applying configuration ([Applying Configuration] (APP))
};
[[nodiscard]] constexpr auto enum_desc(sfdr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case sfdr_e::init:
      return "[Initialization] (INIT), Initialization";
    case sfdr_e::idle:
      return "[Not Active] (IDLE), Not active";
    case sfdr_e::ope:
      return "[Operational] (OPE), Operational";
    case sfdr_e::rdy:
      return "[Ready] (RDY), Ready";
    case sfdr_e::ipc:
      return "[IP Configuration] (IPC), IP configuration";
    case sfdr_e::uncf:
      return "[Not Configured] (UNCF), Not configured";
    case sfdr_e::get:
      return "[Reading Configuration] (GET), Reading configuration";
    case sfdr_e::set:
      return "[Writing Configuration] (SET), Writing configuration";
    case sfdr_e::app:
      return "[Applying Configuration] (APP), Applying configuration";
  }
  return "unknown";
}
constexpr auto format_as(sfdr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of sft_e enum decleration
enum struct sft_e : std::uint16_t {
  hf1 = 1,  ///< Switch.frequency type 1 ([SFR type 1] (HF1))
  hf2 = 2,  ///< Switch.frequency type 2 ([SFR type 2] (HF2))
};
[[nodiscard]] constexpr auto enum_desc(sft_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case sft_e::hf1:
      return "[SFR type 1] (HF1), Switch.frequency type 1";
    case sft_e::hf2:
      return "[SFR type 2] (HF2), Switch.frequency type 2";
  }
  return "unknown";
}
constexpr auto format_as(sft_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of slpm_e enum decleration
enum struct slpm_e : std::uint16_t {
  no = 0,        ///< No ([No] (NO))
  sw = 1,        ///< Switch ([Switch] (SW))
  lf = 2,        ///< Flow ([Flow] (LF))
  spd = 3,       ///< Speed ([Speed] (SPD))
  pwr = 4,       ///< Power ([Power] (PWR))
  hp = 5,        ///< Pressure ([Pressure] (HP))
  multiple = 6,  ///< Multiple ([Multiple] (OR))
};
[[nodiscard]] constexpr auto enum_desc(slpm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case slpm_e::no:
      return "[No] (NO), No";
    case slpm_e::sw:
      return "[Switch] (SW), Switch";
    case slpm_e::lf:
      return "[Flow] (LF), Flow";
    case slpm_e::spd:
      return "[Speed] (SPD), Speed";
    case slpm_e::pwr:
      return "[Power] (PWR), Power";
    case slpm_e::hp:
      return "[Pressure] (HP), Pressure";
    case slpm_e::multiple:
      return "[Multiple] (OR), Multiple";
  }
  return "unknown";
}
constexpr auto format_as(slpm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of smot_e enum decleration
enum struct smot_e : std::uint16_t {
  no = 0,   ///< No information ([No info.] (NO))
  lls = 1,  ///< Low saliency ([Low salient] (LLS))
  mls = 2,  ///< Medium saliency ([Med salient] (MLS))
  hls = 3,  ///< High saliency ([High salient] (HLS))
};
[[nodiscard]] constexpr auto enum_desc(smot_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case smot_e::no:
      return "[No info.] (NO), No information";
    case smot_e::lls:
      return "[Low salient] (LLS), Low saliency";
    case smot_e::mls:
      return "[Med salient] (MLS), Medium saliency";
    case smot_e::hls:
      return "[High salient] (HLS), High saliency";
  }
  return "unknown";
}
constexpr auto format_as(smot_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of sop_e enum decleration
enum struct sop_e : std::uint16_t {
  microseconds_6 = 6,    ///< 6 µs ([6 µs] (6))
  microseconds_8 = 8,    ///< 8 µs ([8 µs] (8))
  microseconds_10 = 10,  ///< 10 µs ([10 µs] (10))
};
[[nodiscard]] constexpr auto enum_desc(sop_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case sop_e::microseconds_6:
      return "[6 µs] (6), 6 µs";
    case sop_e::microseconds_8:
      return "[8 µs] (8), 8 µs";
    case sop_e::microseconds_10:
      return "[10 µs] (10), 10 µs";
  }
  return "unknown";
}
constexpr auto format_as(sop_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ssl_e enum decleration
enum struct ssl_e : std::uint16_t {
  std = 0,  ///< Standard ([Standard] (STD))
  hpf = 1,  ///< High performance ([High Perf] (HPF))
};
[[nodiscard]] constexpr auto enum_desc(ssl_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ssl_e::std:
      return "[Standard] (STD), Standard";
    case ssl_e::hpf:
      return "[High Perf] (HPF), High performance";
  }
  return "unknown";
}
constexpr auto format_as(ssl_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of stos_e enum decleration
enum struct stos_e : std::uint16_t {
  idle = 0,  ///< Not active ([Not active] (IDLE))
  sto = 1,   ///< Active ([Active] (STO))
  flt = 2,   ///< Error ([Error] (FLT))
};
[[nodiscard]] constexpr auto enum_desc(stos_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case stos_e::idle:
      return "[Not active] (IDLE), Not active";
    case stos_e::sto:
      return "[Active] (STO), Active";
    case stos_e::flt:
      return "[Error] (FLT), Error";
  }
  return "unknown";
}
constexpr auto format_as(stos_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of stp_e enum decleration
enum struct stp_e : std::uint16_t {
  no = 0,   ///< Inactive ([Inactive] (NO))
  mms = 1,  ///< Maintain DC Bus  ([Maintain DC Bus] (MMS))
  rmp = 2,  ///< Ramp Stop ([Ramp Stop] (RMP))
  lnf = 4,  ///< Locked in freewheel stop without error ([Freewheel Stop] (LNF))
};
[[nodiscard]] constexpr auto enum_desc(stp_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case stp_e::no:
      return "[Inactive] (NO), Inactive";
    case stp_e::mms:
      return "[Maintain DC Bus] (MMS), Maintain DC Bus ";
    case stp_e::rmp:
      return "[Ramp Stop] (RMP), Ramp Stop";
    case stp_e::lnf:
      return "[Freewheel Stop] (LNF), Locked in freewheel stop without error";
  }
  return "unknown";
}
constexpr auto format_as(stp_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of str_e enum decleration
enum struct str_e : std::uint16_t {
  no = 0,   ///< No save ([No Save] (NO))
  ram = 1,  ///< Save to RAM ([Save to RAM] (RAM))
  eep = 2,  ///< Save to EEPROM ([Save to EEPROM] (EEP))
};
[[nodiscard]] constexpr auto enum_desc(str_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case str_e::no:
      return "[No Save] (NO), No save";
    case str_e::ram:
      return "[Save to RAM] (RAM), Save to RAM";
    case str_e::eep:
      return "[Save to EEPROM] (EEP), Save to EEPROM";
  }
  return "unknown";
}
constexpr auto format_as(str_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of stt_e enum decleration
enum struct stt_e : std::uint16_t {
  rmp = 0,  ///< On ramp ([On Ramp] (RMP))
  fst = 1,  ///< Fast stop ([Fast stop] (FST))
  nst = 2,  ///< Freewheel stop ([Freewheel Stop] (NST))
  dci = 3,  ///< DC injection ([DC injection] (DCI))
};
[[nodiscard]] constexpr auto enum_desc(stt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case stt_e::rmp:
      return "[On Ramp] (RMP), On ramp";
    case stt_e::fst:
      return "[Fast stop] (FST), Fast stop";
    case stt_e::nst:
      return "[Freewheel Stop] (NST), Freewheel stop";
    case stt_e::dci:
      return "[DC injection] (DCI), DC injection";
  }
  return "unknown";
}
constexpr auto format_as(stt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of stun_e enum decleration
enum struct stun_e : std::uint16_t {
  tab = 0,   ///< Default ([Default] (TAB))
  meas = 1,  ///< Measure ([Measure] (MEAS))
  cus = 2,   ///< Custom ([Custom] (CUS))
};
[[nodiscard]] constexpr auto enum_desc(stun_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case stun_e::tab:
      return "[Default] (TAB), Default";
    case stun_e::meas:
      return "[Measure] (MEAS), Measure";
    case stun_e::cus:
      return "[Custom] (CUS), Custom";
  }
  return "unknown";
}
constexpr auto format_as(stun_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of sucu_e enum decleration
enum struct sucu_e : std::uint16_t {
  euro = 0,    ///< Euro ([Euro] (EURO))
  dollar = 1,  ///< $ ([$] (DOLLAR))
  pound = 2,   ///< £ ([£] (POUND))
  kr = 3,      ///< Krone ([Krone] (KR))
  rmb = 4,     ///< Renminbi ([Renminbi] (RMB))
  other = 5,   ///< Other ([Other] (OTHER))
};
[[nodiscard]] constexpr auto enum_desc(sucu_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case sucu_e::euro:
      return "[Euro] (EURO), Euro";
    case sucu_e::dollar:
      return "[$] (DOLLAR), $";
    case sucu_e::pound:
      return "[£] (POUND), £";
    case sucu_e::kr:
      return "[Krone] (KR), Krone";
    case sucu_e::rmb:
      return "[Renminbi] (RMB), Renminbi";
    case sucu_e::other:
      return "[Other] (OTHER), Other";
  }
  return "unknown";
}
constexpr auto format_as(sucu_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of sufr_e enum decleration
enum struct sufr_e : std::uint16_t {
  l_per_s = 0,        ///< 1 L/s ([1 L/s] (1LS))
  decil_per_s = 1,    ///< 0.1 l/s ([0.1 l/s] (01LS))
  l_per_m = 2,        ///< 1 L/mn ([1 L/mn] (1LM))
  l_per_h = 3,        ///< 1 L/h ([1 L/h] (1LH))
  dm3_per_m = 4,      ///< 1 dm3/mn ([1 dm3/mn] (1DM3M))
  m3_per_s = 5,       ///< 1 m3/s ([1 m3/s] (1M3S))
  decim3_per_s = 6,   ///< 0.1 m3/s ([0.1 m3/s] (01M3S))
  m3_per_mn = 7,      ///< 1 m3/mn ([1 m3/mn] (1M3MN))
  decim3_per_mn = 8,  ///< 0.1 m3/mn ([0.1 m3/mn] (01M3MN))
  m3_per_h = 9,       ///< 1 m3/h ([1 m3/h] (1M3H))
  decim3_per_h = 10,  ///< 0.1 m3/h ([0.1 m3/h] (01M3H))
  g_per_s = 11,       ///< 1 gal/s ([1 gal/s] (1GPS))
  g_per_m = 12,       ///< 1 GPM ([1 GPM] (1GPM))
  g_per_h = 13,       ///< 1 gal/h ([1 gal/h] (1GPH))
  cf_per_s = 14,      ///< 1 ft3/s ([1 ft3/s] (1CFS))
  cf_per_m = 15,      ///< 1 CFM ([1 CFM] (1CFM))
  scf_per_m = 16,     ///< 1 SCFM ([1 SCFM] (1SCFM))
  cf_per_h = 17,      ///< 1 ft3/h ([1 ft3/h] (1CFH))
  kg_per_s = 18,      ///< 1 Kg/s ([1 Kg/s] (1KGS))
  kg_per_m = 19,      ///< 1 Kg/mn ([1 Kg/mn] (1KGM))
  kg_per_h = 20,      ///< 1 Kg/h ([1 Kg/h] (1KGH))
  lb_per_s = 21,      ///< 1 Lb/s ([1 Lb/s] (1LBS))
  lb_per_m = 22,      ///< 1 Lb/mn ([1 Lb/mn] (1LBM))
  lb_per_h = 23,      ///< 1 Lb/h ([1 Lb/h] (1LBH))
  decipc = 24,        ///< 0.1 % ([0.1 %] (01PC))
  deciwo = 25,        ///< 0.1 ([0.1] (01WO))
};
[[nodiscard]] constexpr auto enum_desc(sufr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case sufr_e::l_per_s:
      return "[1 L/s] (1LS), 1 L/s";
    case sufr_e::decil_per_s:
      return "[0.1 l/s] (01LS), 0.1 l/s";
    case sufr_e::l_per_m:
      return "[1 L/mn] (1LM), 1 L/mn";
    case sufr_e::l_per_h:
      return "[1 L/h] (1LH), 1 L/h";
    case sufr_e::dm3_per_m:
      return "[1 dm3/mn] (1DM3M), 1 dm3/mn";
    case sufr_e::m3_per_s:
      return "[1 m3/s] (1M3S), 1 m3/s";
    case sufr_e::decim3_per_s:
      return "[0.1 m3/s] (01M3S), 0.1 m3/s";
    case sufr_e::m3_per_mn:
      return "[1 m3/mn] (1M3MN), 1 m3/mn";
    case sufr_e::decim3_per_mn:
      return "[0.1 m3/mn] (01M3MN), 0.1 m3/mn";
    case sufr_e::m3_per_h:
      return "[1 m3/h] (1M3H), 1 m3/h";
    case sufr_e::decim3_per_h:
      return "[0.1 m3/h] (01M3H), 0.1 m3/h";
    case sufr_e::g_per_s:
      return "[1 gal/s] (1GPS), 1 gal/s";
    case sufr_e::g_per_m:
      return "[1 GPM] (1GPM), 1 GPM";
    case sufr_e::g_per_h:
      return "[1 gal/h] (1GPH), 1 gal/h";
    case sufr_e::cf_per_s:
      return "[1 ft3/s] (1CFS), 1 ft3/s";
    case sufr_e::cf_per_m:
      return "[1 CFM] (1CFM), 1 CFM";
    case sufr_e::scf_per_m:
      return "[1 SCFM] (1SCFM), 1 SCFM";
    case sufr_e::cf_per_h:
      return "[1 ft3/h] (1CFH), 1 ft3/h";
    case sufr_e::kg_per_s:
      return "[1 Kg/s] (1KGS), 1 Kg/s";
    case sufr_e::kg_per_m:
      return "[1 Kg/mn] (1KGM), 1 Kg/mn";
    case sufr_e::kg_per_h:
      return "[1 Kg/h] (1KGH), 1 Kg/h";
    case sufr_e::lb_per_s:
      return "[1 Lb/s] (1LBS), 1 Lb/s";
    case sufr_e::lb_per_m:
      return "[1 Lb/mn] (1LBM), 1 Lb/mn";
    case sufr_e::lb_per_h:
      return "[1 Lb/h] (1LBH), 1 Lb/h";
    case sufr_e::decipc:
      return "[0.1 %] (01PC), 0.1 %";
    case sufr_e::deciwo:
      return "[0.1] (01WO), 0.1";
  }
  return "unknown";
}
constexpr auto format_as(sufr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of supr_e enum decleration
enum struct supr_e : std::uint16_t {
  kpa = 0,       ///< 1 Kpa ([1 Kpa] (1KPA))
  mbar = 1,      ///< 1 mbar ([1 mbar] (1MBAR))
  bar = 2,       ///< 1 Bar ([1 Bar] (1BAR))
  decibar = 3,   ///< 0.1 Bar ([0.1 Bar] (01BAR))
  centibar = 4,  ///< 0.01 Bar ([0.01 Bar] (001BAR))
  psi = 5,       ///< 1 Psi ([1 Psi] (1PSI))
  decipsi = 6,   ///< 0.1 Psi ([0.1 Psi] (01PSI))
  psig = 7,      ///< 1 Psig ([1 Psig] (1PSIG))
  decipsig = 8,  ///< 0.1 Psig ([0.1 Psig] (01PSIG))
  inh20 = 9,     ///< 1 inH2O ([1 inH2O] (1INH20))
  inwg = 10,     ///< 1 inWg ([1 inWg] (1INWG))
  inwc = 11,     ///< 1 inWC ([1 inWC] (1INWC))
  ftwg = 12,     ///< 1 ftWg ([1 ftWg] (1FTWG))
  ftwc = 13,     ///< 1 ftWc ([1 ftWc] (1FTWC))
  ft = 14,       ///< 1 ft ([1 ft] (1FT))
  mwg = 15,      ///< 1 mWg ([1 mWg] (1MWG))
  decimwg = 16,  ///< 0.1 mWg ([0.1 mWg] (01MWG))
  mwc = 17,      ///< 1 mWC ([1 mWC] (1MWC))
  decimwc = 18,  ///< 0.1 mWc ([0.1 mWc] (01MWC))
  m = 19,        ///< 1 m ([1 m] (1M))
  decim = 20,    ///< 0.1 m  ([0.1 m ] (01M))
  inhg = 21,     ///< 1 inHg ([1 inHg] (1INHG))
  decipc = 22,   ///< 0.1 % ([0.1 %] (01PC))
  deciwo = 23,   ///< 0.1 ([0.1] (01WO))
};
[[nodiscard]] constexpr auto enum_desc(supr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case supr_e::kpa:
      return "[1 Kpa] (1KPA), 1 Kpa";
    case supr_e::mbar:
      return "[1 mbar] (1MBAR), 1 mbar";
    case supr_e::bar:
      return "[1 Bar] (1BAR), 1 Bar";
    case supr_e::decibar:
      return "[0.1 Bar] (01BAR), 0.1 Bar";
    case supr_e::centibar:
      return "[0.01 Bar] (001BAR), 0.01 Bar";
    case supr_e::psi:
      return "[1 Psi] (1PSI), 1 Psi";
    case supr_e::decipsi:
      return "[0.1 Psi] (01PSI), 0.1 Psi";
    case supr_e::psig:
      return "[1 Psig] (1PSIG), 1 Psig";
    case supr_e::decipsig:
      return "[0.1 Psig] (01PSIG), 0.1 Psig";
    case supr_e::inh20:
      return "[1 inH2O] (1INH20), 1 inH2O";
    case supr_e::inwg:
      return "[1 inWg] (1INWG), 1 inWg";
    case supr_e::inwc:
      return "[1 inWC] (1INWC), 1 inWC";
    case supr_e::ftwg:
      return "[1 ftWg] (1FTWG), 1 ftWg";
    case supr_e::ftwc:
      return "[1 ftWc] (1FTWC), 1 ftWc";
    case supr_e::ft:
      return "[1 ft] (1FT), 1 ft";
    case supr_e::mwg:
      return "[1 mWg] (1MWG), 1 mWg";
    case supr_e::decimwg:
      return "[0.1 mWg] (01MWG), 0.1 mWg";
    case supr_e::mwc:
      return "[1 mWC] (1MWC), 1 mWC";
    case supr_e::decimwc:
      return "[0.1 mWc] (01MWC), 0.1 mWc";
    case supr_e::m:
      return "[1 m] (1M), 1 m";
    case supr_e::decim:
      return "[0.1 m ] (01M), 0.1 m ";
    case supr_e::inhg:
      return "[1 inHg] (1INHG), 1 inHg";
    case supr_e::decipc:
      return "[0.1 %] (01PC), 0.1 %";
    case supr_e::deciwo:
      return "[0.1] (01WO), 0.1";
  }
  return "unknown";
}
constexpr auto format_as(supr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of sutp_e enum decleration
enum struct sutp_e : std::uint16_t {
  decic = 0,   ///< 0.1°C ([0.1°C] (01C))
  decif = 1,   ///< 0.1°F ([0.1°F] (01F))
  decipc = 2,  ///< 0.1 % ([0.1 %] (01PC))
  deciwo = 3,  ///< 0.1 ([0.1] (01WO))
};
[[nodiscard]] constexpr auto enum_desc(sutp_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case sutp_e::decic:
      return "[0.1°C] (01C), 0.1°C";
    case sutp_e::decif:
      return "[0.1°F] (01F), 0.1°F";
    case sutp_e::decipc:
      return "[0.1 %] (01PC), 0.1 %";
    case sutp_e::deciwo:
      return "[0.1] (01WO), 0.1";
  }
  return "unknown";
}
constexpr auto format_as(sutp_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of tbr_e enum decleration
enum struct tbr_e : std::uint16_t {
  not_config = 0,   ///< Automatic ([Automatic] (AUTO))
  automatic = 4,    ///< 300 bps ([300 bps] (300))
  bps_300 = 8,      ///< 600 bps ([600 bps] (600))
  bps_600 = 12,     ///< 1.2 Kbps ([1.2 Kbps] (1200))
  kbps_1_2 = 16,    ///< 2.4 Kbps ([2.4 Kbps] (2400))
  kbps_2_4 = 20,    ///< 4800 bps ([4800 bps] (4800))
  kbps_4_8 = 24,    ///< 9600 bps ([9600 bps] (9600))
  kbps_9_6 = 28,    ///< 10 Kbps ([10 Kbps] (10000))
  kbps_10 = 30,     ///< 19200 bps ([19200 bps] (19200))
  kbps_19_2 = 32,   ///< 20 Kbps ([20 Kbps] (20000))
  kbps_20 = 34,     ///< 28.8 Kbps ([28.8 Kbps] (28800))
  kbps_28_8 = 35,   ///< 38.4 Kbps ([38.4 Kbps] (38400))
  kbps_38_4 = 36,   ///< 45.45 Kbps ([45.45 Kbps] (45450))
  kbps_45_45 = 37,  ///< 50 Kbps ([50 Kbps] (50000))
  kbps_50 = 38,     ///< 57.6 Kbps ([57.6 Kbps] (57600))
  kbps_57_6 = 40,   ///< 76.8 Kbps ([76.8 Kbps] (76800))
  kbps_93_75 = 42,  ///< 93.75 Kbps ([93.75 Kbps] (93750))
  kbps_100 = 44,    ///< 100 Kbps ([100 Kbps] (100K))
  kbps_115_2 = 48,  ///< 115.2 Kbps ([115.2 Kbps] (115K2))
  kbps_125 = 52,    ///< 125 Kbps ([125 Kbps] (125K))
  kbps_156 = 53,    ///< 156 Kbps ([156 Kbps] (156K))
  kbps_187_5 = 54,  ///< 187.5 Kbps ([187.5 Kbps] (187K5))
  kbps_230_4 = 56,  ///< 230.4 Kbps ([230.4 Kbps] (230K4))
  kbps_250 = 60,    ///< 250 Kbps ([250 Kbps] (250K))
  kbps_460_8 = 64,  ///< 460.8 Kbps ([460.8 Kbps] (460K8))
  kbps_500 = 68,    ///< 500 Kbps ([500 Kbps] (500K))
  kbps_625 = 69,    ///< 625 Kbps ([625 Kbps] (625K))
  kbps_800 = 70,    ///< 800 Kbps ([800 Kbps] (800K))
  kbps_921_6 = 72,  ///< 921.6 Kbps ([921.6 Kbps] (921K6))
  mbps_1 = 76,      ///< 1 Mbps ([1 Mbps] (1M))
  mbps_1_5 = 80,    ///< 1.5 Mbps ([1.5 Mbps] (1M5))
  mbps_2_5 = 81,    ///< 2.5 Mbps ([2.5 Mbps] (2M5))
  mbps_3 = 82,      ///< 3 Mbps ([3 Mbps] (3M))
  mbps_6 = 83,      ///< 6 Mbps ([6 Mbps] (6M))
  mbps_10 = 84,     ///< 10 Mbps ([10 Mbps] (10M))
  mbps_5 = 86,      ///< 5 Mbps ([5 Mbps] (5M))
  mbps_12 = 88,     ///< 12 Mbps ([12 Mbps] (12M))
  mbps_100 = 92,    ///< 100 Mbps ([100 Mbps] (100M))
};
[[nodiscard]] constexpr auto enum_desc(tbr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case tbr_e::not_config:
      return "[Not config.] (nO), ";
    case tbr_e::automatic:
      return "[Automatic] (AUtO), ";
    case tbr_e::bps_300:
      return "[300 bps] (300), ";
    case tbr_e::bps_600:
      return "[600 bps] (600), ";
    case tbr_e::kbps_1_2:
      return "[1.2 Kbps] (1 2), ";
    case tbr_e::kbps_2_4:
      return "[2.4 Kbps] (2 4), ";
    case tbr_e::kbps_4_8:
      return "[4.8 Kbps] (4 8), ";
    case tbr_e::kbps_9_6:
      return "[9.6 Kbps] (9 6), ";
    case tbr_e::kbps_10:
      return "[10 Kbps] (10 ), ";
    case tbr_e::kbps_19_2:
      return "[19.2 Kbps] (19 2), ";
    case tbr_e::kbps_20:
      return "[20 Kbps] (20 ), ";
    case tbr_e::kbps_28_8:
      return "[28.8 Kbps] (28 8), ";
    case tbr_e::kbps_38_4:
      return "[38.4 Kbps] (38 4), ";
    case tbr_e::kbps_45_45:
      return "[45.45 Kbps] (45 4), ";
    case tbr_e::kbps_50:
      return "[50 Kbps] (50 ), ";
    case tbr_e::kbps_57_6:
      return "[57.6 Kbps] (57 6), ";
    case tbr_e::kbps_93_75:
      return "[93.75 Kbps] (93 7), ";
    case tbr_e::kbps_100:
      return "[100 Kbps] (100 ), ";
    case tbr_e::kbps_115_2:
      return "[115.2 Kbps] (115 ), ";
    case tbr_e::kbps_125:
      return "[125 Kbps] (125 ), ";
    case tbr_e::kbps_156:
      return "[156 Kbps] (156 ), ";
    case tbr_e::kbps_187_5:
      return "[187.5 Kbps] (187 ), ";
    case tbr_e::kbps_230_4:
      return "[230.4 Kbps] (230 ), ";
    case tbr_e::kbps_250:
      return "[250 Kbps] (250 ), ";
    case tbr_e::kbps_460_8:
      return "[460.8 Kbps] (460 ), ";
    case tbr_e::kbps_500:
      return "[500 Kbps] (500 ), ";
    case tbr_e::kbps_625:
      return "[625 Kbps] (625 ), ";
    case tbr_e::kbps_800:
      return "[800 Kbps] (800 ), ";
    case tbr_e::kbps_921_6:
      return "[921.6 Kbps] (921 ), ";
    case tbr_e::mbps_1:
      return "[1 Mbps] (1M), ";
    case tbr_e::mbps_1_5:
      return "[1.5 Mbps] (1M5), ";
    case tbr_e::mbps_2_5:
      return "[2.5 Mbps] (2M5), ";
    case tbr_e::mbps_3:
      return "[3 Mbps] (3M), ";
    case tbr_e::mbps_6:
      return "[6 Mbps] (6M), ";
    case tbr_e::mbps_10:
      return "[10 Mbps] (10M), ";
    case tbr_e::mbps_5:
      return "[5 Mbps] (5M), ";
    case tbr_e::mbps_12:
      return "[12 Mbps] (12M), ";
    case tbr_e::mbps_100:
      return "[100 Mbps] (100M), ";
  }
  return "unknown";
}
constexpr auto format_as(tbr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of tcc_e enum decleration
enum struct tcc_e : std::uint16_t {
  two_wire_ctrl = 0,    ///< 2-wire control ([2-Wire Control] (2C))
  three_wire_ctrl = 1,  ///< 3-wire control ([3-Wire Control] (3C))
};
[[nodiscard]] constexpr auto enum_desc(tcc_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case tcc_e::two_wire_ctrl:
      return "[2-Wire Control] (2C), 2-wire control";
    case tcc_e::three_wire_ctrl:
      return "[3-Wire Control] (3C), 3-wire control";
  }
  return "unknown";
}
constexpr auto format_as(tcc_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of tct_e enum decleration
enum struct tct_e : std::uint16_t {
  lel = 0,  ///< Level ([Level] (LEL))
  trn = 1,  ///< Transition ([Transition] (TRN))
  pfo = 2,  ///< Level with forward priority ([Level With Fwd Priority] (PFO))
};
[[nodiscard]] constexpr auto enum_desc(tct_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case tct_e::lel:
      return "[Level] (LEL), Level";
    case tct_e::trn:
      return "[Transition] (TRN), Transition";
    case tct_e::pfo:
      return "[Level With Fwd Priority] (PFO), Level with forward priority";
  }
  return "unknown";
}
constexpr auto format_as(tct_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of tht_e enum decleration
enum struct tht_e : std::uint16_t {
  no = 0,   ///< No thermal monitoring ([No] (NO))
  acl = 1,  ///< Self cooled motor ([Self cooled] (ACL))
  fcl = 2,  ///< Force cooled motor ([Force-cool] (FCL))
};
[[nodiscard]] constexpr auto enum_desc(tht_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case tht_e::no:
      return "[No] (NO), No thermal monitoring";
    case tht_e::acl:
      return "[Self cooled] (ACL), Self cooled motor";
    case tht_e::fcl:
      return "[Force-cool] (FCL), Force cooled motor";
  }
  return "unknown";
}
constexpr auto format_as(tht_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of toct_e enum decleration
enum struct toct_e : std::uint16_t {
  na = 0,     ///< NA ([NA] (NA))
  press = 1,  ///< PRESSURE ([PRESSURE] (PRESS))
  flow = 2,   ///< FLOW ([FLOW] (FLOW))
  other = 3,  ///< OTHER ([OTHER] (OTHER))
};
[[nodiscard]] constexpr auto enum_desc(toct_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case toct_e::na:
      return "[NA] (NA), NA";
    case toct_e::press:
      return "[PRESSURE] (PRESS), PRESSURE";
    case toct_e::flow:
      return "[FLOW] (FLOW), FLOW";
    case toct_e::other:
      return "[OTHER] (OTHER), OTHER";
  }
  return "unknown";
}
constexpr auto format_as(toct_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of tunt_e enum decleration
enum struct tunt_e : std::uint16_t {
  std = 2,  ///< Standard ([Standard] (STD))
  rot = 3,  ///< Rotation ([Rotation] (ROT))
};
[[nodiscard]] constexpr auto enum_desc(tunt_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case tunt_e::std:
      return "[Standard] (STD), Standard";
    case tunt_e::rot:
      return "[Rotation] (ROT), Rotation";
  }
  return "unknown";
}
constexpr auto format_as(tunt_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of tunu_e enum decleration
enum struct tunu_e : std::uint16_t {
  no = 0,  ///< No ([No] (NO))
  tm = 1,  ///< Use the motor thermal evolution ([Therm mot] (TM))
};
[[nodiscard]] constexpr auto enum_desc(tunu_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case tunu_e::no:
      return "[No] (NO), No";
    case tunu_e::tm:
      return "[Therm mot] (TM), Use the motor thermal evolution";
  }
  return "unknown";
}
constexpr auto format_as(tunu_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of two_e enum decleration
enum struct two_e : std::uint16_t {
  low = 0,   ///< ModBus Word Order OFF ([OFF] (LOW))
  high = 1,  ///< Modbus Word Order ON ([ON] (HIGH))
};
[[nodiscard]] constexpr auto enum_desc(two_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case two_e::low:
      return "[OFF] (LOW), ModBus Word Order OFF";
    case two_e::high:
      return "[ON] (HIGH), Modbus Word Order ON";
  }
  return "unknown";
}
constexpr auto format_as(two_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ulr_e enum decleration
enum struct ulr_e : std::uint16_t {
  ulr0 = 0,  ///< Upload access allowed ([Permitted] (ULR0))
  ulr1 = 1,  ///< Upload access not allow ([Not allowed] (ULR1))
};
[[nodiscard]] constexpr auto enum_desc(ulr_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ulr_e::ulr0:
      return "[Permitted] (ULR0), Upload access allowed";
    case ulr_e::ulr1:
      return "[Not allowed] (ULR1), Upload access not allow";
  }
  return "unknown";
}
constexpr auto format_as(ulr_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of ures_e enum decleration
enum struct ures_e : std::uint16_t {
  ac200V = 20,  ///< 200 Vac ([200 Vac] (200))
  ac220V = 22,  ///< 220 Vac ([220 Vac] (220))
  ac230V = 23,  ///< 230 Vac ([230 Vac] (230))
  ac240V = 24,  ///< 240 Vac ([240 Vac] (240))
  ac380V = 38,  ///< 380 Vac ([380 Vac] (380))
  ac400V = 40,  ///< 400 Vac ([400 Vac] (400))
  ac440V = 44,  ///< 440 Vac ([440 Vac] (440))
  ac460V = 46,  ///< 460 Vac ([460 Vac] (460))
  ac480V = 48,  ///< 480 Vac ([480 Vac] (480))
  ac500V = 50,  ///< 500Vac ([500Vac] (500))
  ac525V = 52,  ///< 525 Vac ([525 Vac] (525))
  ac575V = 57,  ///< 575 Vac ([575 Vac] (575))
  ac600V = 60,  ///< 600 Vac ([600 Vac] (600))
  ac690V = 69,  ///< 690 Vac ([690 Vac] (690))
};
[[nodiscard]] constexpr auto enum_desc(ures_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case ures_e::ac200V:
      return "[200 Vac] (200), 200 Vac";
    case ures_e::ac220V:
      return "[220 Vac] (220), 220 Vac";
    case ures_e::ac230V:
      return "[230 Vac] (230), 230 Vac";
    case ures_e::ac240V:
      return "[240 Vac] (240), 240 Vac";
    case ures_e::ac380V:
      return "[380 Vac] (380), 380 Vac";
    case ures_e::ac400V:
      return "[400 Vac] (400), 400 Vac";
    case ures_e::ac440V:
      return "[440 Vac] (440), 440 Vac";
    case ures_e::ac460V:
      return "[460 Vac] (460), 460 Vac";
    case ures_e::ac480V:
      return "[480 Vac] (480), 480 Vac";
    case ures_e::ac500V:
      return "[500Vac] (500), 500Vac";
    case ures_e::ac525V:
      return "[525 Vac] (525), 525 Vac";
    case ures_e::ac575V:
      return "[575 Vac] (575), 575 Vac";
    case ures_e::ac600V:
      return "[600 Vac] (600), 600 Vac";
    case ures_e::ac690V:
      return "[690 Vac] (690), 690 Vac";
  }
  return "unknown";
}
constexpr auto format_as(ures_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of usb_e enum decleration
enum struct usb_e : std::uint16_t {
  error_triggered = 0,                 ///< Error triggered ([Error Triggered] (0))
  error_triggered_with_out_relay = 1,  ///< Error triggered w/o relay ([Error Triggered w/o Relay] (1))
  warning_triggered = 2,               ///< Warning Triggered ([Warning Triggered] (2))
};
[[nodiscard]] constexpr auto enum_desc(usb_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case usb_e::error_triggered:
      return "[Error Triggered] (0), Error triggered";
    case usb_e::error_triggered_with_out_relay:
      return "[Error Triggered w/o Relay] (1), Error triggered w/o relay";
    case usb_e::warning_triggered:
      return "[Warning Triggered] (2), Warning Triggered";
  }
  return "unknown";
}
constexpr auto format_as(usb_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of vcal_e enum decleration
enum struct vcal_e : std::uint16_t {
  no = 0,                    ///< Unkown voltage ([Unknown Voltage] (NO))
  single_phase_100_120 = 1,  ///< 100-120 V single phase ([100-120(1)] (110M))
  three_phase_100_120 = 2,   ///< 100-120 V three phase ([100-120(3)] (110T))
  single_phase_200_240 = 3,  ///< 200-240 V single ([200-240 V single] (220M))
  three_phase_200_240 = 4,   ///< 200-240 V three phase ([200-240 V Three] (220T))
  single_phase_380_500 = 5,  ///< 380-500 V single phase ([380-500(1)] (480M))
  three_phase_380_500 = 6,   ///< 380-500 V three phase ([380-500 V Three] (480T))
  single_phase_500_690 = 7,  ///< 500-690 V single phase ([500-690(1)] (690M))
  three_phase_500_690 = 8,   ///< 500-690 V three phase ([500-690 V Three] (690T))
  single_phase_600 = 9,      ///< 600 V single phase ([600(1)] (600M))
  three_phase_600 = 10,      ///< 600 V three phase ([600 V Three] (600T))
};
[[nodiscard]] constexpr auto enum_desc(vcal_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case vcal_e::no:
      return "[Unknown Voltage] (NO), Unkown voltage";
    case vcal_e::single_phase_100_120:
      return "[100-120(1)] (110M), 100-120 V single phase";
    case vcal_e::three_phase_100_120:
      return "[100-120(3)] (110T), 100-120 V three phase";
    case vcal_e::single_phase_200_240:
      return "[200-240 V single] (220M), 200-240 V single";
    case vcal_e::three_phase_200_240:
      return "[200-240 V Three] (220T), 200-240 V three phase";
    case vcal_e::single_phase_380_500:
      return "[380-500(1)] (480M), 380-500 V single phase";
    case vcal_e::three_phase_380_500:
      return "[380-500 V Three] (480T), 380-500 V three phase";
    case vcal_e::single_phase_500_690:
      return "[500-690(1)] (690M), 500-690 V single phase";
    case vcal_e::three_phase_500_690:
      return "[500-690 V Three] (690T), 500-690 V three phase";
    case vcal_e::single_phase_600:
      return "[600(1)] (600M), 600 V single phase";
    case vcal_e::three_phase_600:
      return "[600 V Three] (600T), 600 V three phase";
  }
  return "unknown";
}
constexpr auto format_as(vcal_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

// Begin of wupm_e enum decleration
enum struct wupm_e : std::uint16_t {
  fbk = 0,  ///< Feedback ([Feedback] (FBK))
  err = 1,  ///< Error ([Error] (ERR))
  lp = 2,   ///< Pressure ([Pressure] (LP))
};
[[nodiscard]] constexpr auto enum_desc(wupm_e const enum_value) -> std::string_view {
  switch (enum_value) {
    case wupm_e::fbk:
      return "[Feedback] (FBK), Feedback";
    case wupm_e::err:
      return "[Error] (ERR), Error";
    case wupm_e::lp:
      return "[Pressure] (LP), Pressure";
  }
  return "unknown";
}
constexpr auto format_as(wupm_e const enum_value) -> std::string_view {
  return enum_desc(enum_value);
}

}  // namespace tfc::ec::devices::schneider::atv320

template <>
struct glz::meta<tfc::ec::devices::schneider::atv320::ecfg_e> {
  static constexpr std::string_view name{ "ecfg" };
  using enum tfc::ec::devices::schneider::atv320::ecfg_e;
  static constexpr auto value{ glz::enumerate(enum_desc(no),
                                              no,
                                              enum_desc(yes),
                                              yes,
                                              enum_desc(stt),
                                              stt,
                                              enum_desc(lff),
                                              lff,
                                              enum_desc(rls),
                                              rls,
                                              enum_desc(rmp),
                                              rmp,
                                              enum_desc(fst),
                                              fst,
                                              enum_desc(dci),
                                              dci) };
};
