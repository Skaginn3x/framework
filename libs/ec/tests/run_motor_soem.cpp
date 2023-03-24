/** \file
 * \brief Example code for Simple Open EtherCAT master
 *
 * Usage: simple_ng IFNAME1
 * IFNAME1 is the NIC interface name, e.g. 'eth0'
 *
 * This is a minimal test.
 */

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ranges>

#include "ethercat.h"

#include "tfc/cia/402.hpp"
#include "tfc/ecx.hpp"

namespace ecx {}

class atv320 {
public:
  static constexpr auto vendor_id = 0x0800005a;
  static constexpr auto product_code = 0x00000389;

  static auto setup(ecx_contextt* context, uint16_t slave) -> int {
    /*
     * IO - LAYOUT
     * INPUT
     * CONTROL WORD        - bit field = 0x6040
     * REFERENCE FREQUENCY - uint16
     * OL1R - Logical output states
     *
     * OUTPUT
     * STATUS WORD       - bit field = 0x6041
     * CURRENT FREQUENCY - uint16 =
     * CURRENT (A)
     * DI - bitfield ( DI1 - DI6 )
     * AI3
     *
     * POLLED ON CONDITION
     * LFT - LAST ERROR - ENUM - WHEN THERE IS FAULT
     * OPR - MOTOR POWER
     * HMIS - STATUS OF DRIVE
     * HSP - High speed set point
     * LSP - Low speed set point
     */

    // Set PDO variables
    // Clean rx and tx prod assign
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_assign<0x00>, 0);
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_assign<0x00>, 0);

    // Zero the size
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_mapping<0x00>, 0);
    // Assign rx variables
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x01>, 0x60410010);  // ETA  - STATUS WORD
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x02>, 0x20020310);  // RFR  - CURRENT SPEED HZ
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x03>, 0x20020510);  // LCR  - CURRENT USAGE ( A )
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x04>, 0x20160310);  // 1LIR - DI1-DI6
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x05>, 0x20162B10);  // AI1C - Physical value AI1
    ecx::sdo_write<uint32_t>(context, slave, ecx::rx_pdo_mapping<0x06>, 0x20162D10);  // AI3C - Physical value AI3
    // Set rx size
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_mapping<0x00>, 5);

    // Zero the size
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_mapping<0x00>, 0);
    // Assign tx variables
    ecx::sdo_write<uint32_t>(context, slave, ecx::tx_pdo_mapping<0x01>, 0x60400010);  // CMD - CONTROL WORD
    ecx::sdo_write<uint32_t>(context, slave, ecx::tx_pdo_mapping<0x02>, 0x20370310);  // LFR - REFERENCE SPEED HZ
    ecx::sdo_write<uint32_t>(
        context, slave, ecx::tx_pdo_mapping<0x03>,
        0x20160D10);  // OL1R - Logic outputs states ( bit0: Relay 1, bit1: Relay 2, bit3 - bit7: unknown, bit8: DQ1 )
    ecx::sdo_write<uint32_t>(context, slave, ecx::tx_pdo_mapping<0x03>, 0x20164810);  // AO1C - AQ1 physical value

    // Set tx size
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_mapping<0x00>, 3);

    // Assign pdo's to mappings
    ecx::sdo_write<uint16_t>(context, slave, ecx::rx_pdo_assign<0x01>, ecx::rx_pdo_mapping<>.first);
    ecx::sdo_write<uint8_t>(context, slave, ecx::rx_pdo_assign<0x00>, 1);

    ecx::sdo_write<uint16_t>(context, slave, ecx::tx_pdo_assign<0x01>, ecx::tx_pdo_mapping<>.first);
    ecx::sdo_write<uint8_t>(context, slave, ecx::tx_pdo_assign<0x00>, 1);

    // Clear internal ATV Functionality for outputs and inputs
    // Disconnect relay 1 from fault assignment
    ecx::sdo_write<uint16_t>(context, slave, { 0x2014, 0x02 }, 0);  // 0 - Not configured
    // Disconnect analog output 1 from frequency
    ecx::sdo_write<uint16_t>(context, slave, { 0x2014, 0x16 }, 0);  // 0 - Not configured
    // Set AO1 output to current
    ecx::sdo_write<uint16_t>(context, slave, { 0x2010, 0x02 }, 2);  // 2 - Current
    // Set AI1 input to current
    ecx::sdo_write<uint16_t>(context, slave, { 0x200E, 0x03 }, 2);  // 2 - Current
    return 1;
  }
};

using Fieldbus = struct {
  ecx_contextt context;
  char* iface;
  uint8 group;
  int roundtrip_time;

  /* Used by the context */
  std::array<uint8, 4096> map;
  ecx_portt port;
  std::array<ec_slavet, EC_MAXSLAVE> slavelist;
  int slavecount;
  std::array<ec_groupt, EC_MAXGROUP> grouplist;
  std::array<uint8, EC_MAXEEPBUF> esibuf;
  std::array<uint32, EC_MAXEEPBITMAP> esimap;
  ec_eringt elist;
  ec_idxstackT idxstack;
  boolean ecaterror;
  int64 DCtime;
  ec_SMcommtypet SMcommtype[EC_MAX_MAPT];
  ec_PDOassignt PDOassign[EC_MAX_MAPT];
  ec_PDOdesct PDOdesc[EC_MAX_MAPT];
  ec_eepromSMt eepSM;
  ec_eepromFMMUt eepFMMU;
};

static void fieldbus_initialize(Fieldbus* fieldbus, char* iface) {
  ecx_contextt* context;

  /* Let's start by 0-filling `fieldbus` to avoid surprises */
  memset(fieldbus, 0, sizeof(*fieldbus));

  fieldbus->iface = iface;
  fieldbus->group = 0;
  fieldbus->roundtrip_time = 0;
  fieldbus->ecaterror = FALSE;

  /* Initialize the ecx_contextt data structure */
  context = &fieldbus->context;
  context->port = &fieldbus->port;
  context->slavelist = fieldbus->slavelist.data();
  context->slavecount = &fieldbus->slavecount;
  context->maxslave = EC_MAXSLAVE;
  context->grouplist = fieldbus->grouplist.data();
  context->maxgroup = EC_MAXGROUP;
  context->esibuf = fieldbus->esibuf.data();
  context->esimap = fieldbus->esimap.data();
  context->esislave = 0;
  context->elist = &fieldbus->elist;
  context->idxstack = &fieldbus->idxstack;
  context->ecaterror = &fieldbus->ecaterror;
  context->DCtime = &fieldbus->DCtime;
  context->SMcommtype = fieldbus->SMcommtype;
  context->PDOassign = fieldbus->PDOassign;
  context->PDOdesc = fieldbus->PDOdesc;
  context->eepSM = &fieldbus->eepSM;
  context->eepFMMU = &fieldbus->eepFMMU;
  context->FOEhook = nullptr;
  context->EOEhook = nullptr;
  context->manualstatechange = 0;
}

static int fieldbus_roundtrip(Fieldbus* fieldbus) {
  ecx_contextt* context;
  ec_timet start, end, diff;
  int wkc;

  context = &fieldbus->context;
  start = osal_current_time();
  ecx_send_processdata(context);
  wkc = ecx_receive_processdata(context, EC_TIMEOUTRET);
  end = osal_current_time();
  osal_time_diff(&start, &end, &diff);
  fieldbus->roundtrip_time = diff.sec * 1000000 + diff.usec;

  return wkc;
}

auto fieldbus_start(Fieldbus* fieldbus) -> boolean {
  ecx_contextt* context;
  ec_groupt* grp;
  int i;

  context = &fieldbus->context;
  grp = fieldbus->grouplist.data() + fieldbus->group;

  printf("Initializing SOEM on '%s'... ", fieldbus->iface);
  if (ecx_init(context, fieldbus->iface) == 0) {
    printf("no socket connection\n");
    return FALSE;
  }
  printf("done\n");

  printf("Finding autoconfig slaves... ");
  if (ecx_config_init(context, FALSE) <= 0) {
    printf("no slaves found\n");
    return FALSE;
  }
  printf("%d slaves found\n", fieldbus->slavecount);

  // Configure atv with initalization function
  fieldbus->slavelist[2].PO2SOconfigx = atv320::setup;

  printf("Sequential mapping of I/O... ");
  auto size = ecx_config_map_group(context, fieldbus->map.data(), fieldbus->group);
  printf("io map Size %d", size);
  printf("mapped %dO+%dI bytes from %d segments", grp->Obytes, grp->Ibytes, grp->nsegments);
  if (grp->nsegments > 1) {
    /* Show how slaves are distrubuted */
    for (i = 0; i < grp->nsegments; ++i) {
      printf("%s%d", i == 0 ? " (" : "+", grp->IOsegment[i]);
    }
    printf(" slaves)");
  }
  printf("\n");

  printf("Configuring distributed clock... ");
  ecx_configdc(context);
  printf("done\n");

  printf("Waiting for all slaves in safe operational... ");
  ecx_statecheck(context, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);
  printf("done\n");

  printf("Send a roundtrip to make outputs in slaves happy... ");
  fieldbus_roundtrip(fieldbus);
  printf("done\n");

  printf("Setting operational state..");
  /* Act on slave 0 (a virtual slave used for broadcasting) */
  auto* slave = fieldbus->slavelist.data();
  slave->state = EC_STATE_OPERATIONAL;
  ecx_writestate(context, 0);
  /* Poll the result ten times before giving up */
  for (i = 0; i < 10; ++i) {
    printf(".");
    fieldbus_roundtrip(fieldbus);
    ecx_statecheck(context, 0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE / 10);
    if (slave->state == EC_STATE_OPERATIONAL) {
      printf(" all slaves are now operational\n");
      return TRUE;
    }
  }

  printf(" failed,");
  ecx_readstate(context);
  for (i = 1; i <= fieldbus->slavecount; ++i) {
    slave = fieldbus->slavelist.data() + i;
    if (slave->state != EC_STATE_OPERATIONAL) {
      printf(" slave %d is 0x%04X (AL-status=0x%04X %s)", i, slave->state, slave->ALstatuscode,
             ec_ALstatuscode2string(slave->ALstatuscode));
    }
  }
  printf("\n");

  return FALSE;
}

static void fieldbus_stop(Fieldbus* fieldbus) {
  auto* context = &fieldbus->context;
  /* Act on slave 0 (a virtual slave used for broadcasting) */
  auto* slave = fieldbus->slavelist.data();

  printf("Requesting init state on all slaves... ");
  slave->state = EC_STATE_INIT;
  ecx_writestate(context, 0);
  printf("done\n");

  printf("Close socket... ");
  ecx_close(context);
  printf("done\n");
}

static boolean fieldbus_dump(Fieldbus* fieldbus) {
  ec_groupt* grp;
  int wkc, expected_wkc;

  grp = fieldbus->grouplist.data() + fieldbus->group;

  wkc = fieldbus_roundtrip(fieldbus);
  expected_wkc = grp->outputsWKC * 2 + grp->inputsWKC;
  if (wkc < expected_wkc) {
    printf(" wrong (expected %d)\n", expected_wkc);
    return FALSE;
  }

  // printf("  O:");
  // for (n = 0; n < fieldbus->slavelist[1].Obytes; ++n) {
  //   printf(" %02X", fieldbus->slavelist[1].outputs[n]);
  // }
  // printf("  I:");
  // for (n = 0; n < fieldbus->slavelist[1].Ibytes; ++n) {
  //   printf(" %02X", fieldbus->slavelist[1].inputs[n]);
  // }
  // // printf("  T: %lld\r", static_cast<long long>(fieldbus->DCtime));
  // printf("\n");

  uint8_t ana0 = fieldbus->slavelist[1].inputs[0];
  // uint8_t ana2 = fieldbus->slavelist[1].inputs[1];
  uint8_t di = fieldbus->slavelist[1].inputs[6];  // NOLINT
  fieldbus->slavelist[1].outputs[0] = di;
  printf("di: %x", di);

  auto* status_word = reinterpret_cast<uint16_t*>(fieldbus->slavelist[2].inputs);
  uint16_t* current_trajectory = reinterpret_cast<uint16_t*>(fieldbus->slavelist[2].inputs) + 1;
  uint16_t command = 0;
  auto reference_trajectory = static_cast<uint16_t>(500.0 * ana0 / 0xff);
  auto state = tfc::ec::cia_402::parse_state(*status_word);
  using tfc::ec::cia_402::commands_e;
  using tfc::ec::cia_402::states_e;
  switch (state) {
    case states_e::switch_on_disabled:
      command = commands_e::shutdown;
      break;
    case states_e::ready_to_switch_on:
      command = commands_e::switch_on;
      break;
    case states_e::operation_enabled:
      if (di & 0x01) {
        command = commands_e::enable_operation;
      } else {
        command = commands_e::shutdown;
      }
      // reference_trajectory = max_speed;
      // if (current_trajectory == max_speed && !down){
      //   start = std::chrono::high_resolution_clock::now();
      //   down = true;
      // }
      // if (current_trajectory == 0 && down){
      //   start = std::chrono::high_resolution_clock::now();
      //   down = false;
      // }
      // if (!down)
      //  reference_trajectory = map_time_and_values(std::chrono::high_resolution_clock::now() - start,
      //  std::chrono::milliseconds(500), 0, max_speed);
      // else
      //  reference_trajectory = map_time_and_values(std::chrono::high_resolution_clock::now() - start,
      //  std::chrono::milliseconds(500), max_speed, 0);
      break;
    case states_e::switched_on:
      if (di & 0x01) {
        command = commands_e::enable_operation;
      } else {
        command = commands_e::switch_on;
      }
      // start = std::chrono::high_resolution_clock::now();
      break;
    case states_e::fault:
      command = commands_e::fault_reset;
      break;
    default:
      break;
  }

  printf("Status word 0x%x, command: %x, state: '%s' ref_speed: %d current_speed: %d\n", *status_word, command, tfc::ec::cia_402::to_string(state).c_str(),
         reference_trajectory, *current_trajectory);
  auto* data_ptr = reinterpret_cast<uint16_t*>(fieldbus->slavelist[2].outputs);
  *data_ptr++ = command;
  *data_ptr++ = reference_trajectory;
  return TRUE;
}

static void fieldbus_check_state(Fieldbus* fieldbus) {
  ecx_contextt* context;
  ec_groupt* grp;
  ec_slavet* slave;
  int i;

  context = &fieldbus->context;
  grp = context->grouplist + fieldbus->group;
  grp->docheckstate = FALSE;
  ecx_readstate(context);
  for (i = 1; i <= fieldbus->slavecount; ++i) {
    slave = context->slavelist + i;
    if (slave->group != fieldbus->group) {
      /* This slave is part of another group: do nothing */
    } else if (slave->state != EC_STATE_OPERATIONAL) {
      grp->docheckstate = TRUE;
      if (slave->state == EC_STATE_SAFE_OP + EC_STATE_ERROR) {
        printf("* Slave %d is in SAFE_OP+ERROR, attempting ACK\n", i);
        slave->state = EC_STATE_SAFE_OP + EC_STATE_ACK;
        ecx_writestate(context, i);
      } else if (slave->state == EC_STATE_SAFE_OP) {
        printf("* Slave %d is in SAFE_OP, change to OPERATIONAL\n", i);
        slave->state = EC_STATE_OPERATIONAL;
        ecx_writestate(context, i);
      } else if (slave->state > EC_STATE_NONE) {
        if (ecx_reconfig_slave(context, i, EC_TIMEOUTRET)) {
          slave->islost = FALSE;
          printf("* Slave %d reconfigured\n", i);
        }
      } else if (!slave->islost) {
        ecx_statecheck(context, i, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
        if (slave->state == EC_STATE_NONE) {
          slave->islost = TRUE;
          printf("* Slave %d lost\n", i);
        }
      }
    } else if (slave->islost) {
      if (slave->state != EC_STATE_NONE) {
        slave->islost = FALSE;
        printf("* Slave %d found\n", i);
      } else if (ecx_recover_slave(context, i, EC_TIMEOUTRET)) {
        slave->islost = FALSE;
        printf("* Slave %d recovered\n", i);
      }
    }
  }

  if (!grp->docheckstate) {
    printf("All slaves resumed OPERATIONAL\n");
  }
}

auto main(int argc, char* argv[]) -> int {
  Fieldbus fieldbus;
  printf("fieldbus size %lu\n", sizeof(fieldbus));
  if (argc != 2) {
    ec_adaptert* adapter = nullptr;
    printf(
        "Usage: simple_ng IFNAME1\n"
        "IFNAME1 is the NIC interface name, e.g. 'eth0'\n");

    printf("\nAvailable adapters:\n");
    adapter = ec_find_adapters();
    while (adapter != nullptr) {
      printf("    - %s  (%s)\n", adapter->name, adapter->desc);
      adapter = adapter->next;
    }
    ec_free_adapters(adapter);
    return 1;
  }

  fieldbus_initialize(&fieldbus, argv[1]);
  if (fieldbus_start(&fieldbus)) {
    int i, min_time, max_time;
    min_time = max_time = 0;
    for (i = 1; i <= 100000; ++i) {
      // printf("Iteration %4d:", i);
      if (!fieldbus_dump(&fieldbus)) {
        fieldbus_check_state(&fieldbus);
      } else if (i == 1) {
        min_time = max_time = fieldbus.roundtrip_time;
      } else if (fieldbus.roundtrip_time < min_time) {
        min_time = fieldbus.roundtrip_time;
      } else if (fieldbus.roundtrip_time > max_time) {
        max_time = fieldbus.roundtrip_time;
      }
      osal_usleep(5000);
    }
    fieldbus_stop(&fieldbus);
  }

  return 0;
}