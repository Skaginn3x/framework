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
#include <cstring>
#include <ranges>

#include "ethercat.h"

#include "tfc/cia/402.hpp"
#include "tfc/ec.hpp"
#include "tfc/devices/schneider/atv320.hpp"


auto fieldbus_roundtrip(tfc::ec::context_t& ec_ctx) -> ecx::working_counter_t {
  return ec_ctx.processdata(ecx::constants::timeout_tx_to_rx);
}

auto fieldbus_start(tfc::ec::context_t& ec_ctx) -> bool {
  printf("Initializing SOEM on '%s'... ", ec_ctx.iface().data());
  if (!ec_ctx.init()) {
    printf("no socket connection\n");
    return false;
  }
  printf("done\n");

  printf("Finding autoconfig slaves... ");
  if (!ec_ctx.config_init(false)) {
    printf("no slaves found\n");
    return false;
  }
  printf("%zu slaves found\n", ec_ctx.slave_count());

  printf("\nMapping of I/O...\n\n");
  auto group_index = 0; // All groups
  size_t const iomapsize = ec_ctx.config_overlap_map_group(group_index);
  printf("\nIOMap size: %zu\n", iomapsize);

  printf("Configuring distributed clock... ");
  ec_ctx.configdc();
  printf("done\n");

  printf("Waiting for all slaves in safe operational... ");
  ec_ctx.statecheck(0, EC_STATE_SAFE_OP, ecx::constants::timeout_state *4);
  printf("done\n");

  printf("Send a roundtrip to make outputs in slaves happy... ");
  fieldbus_roundtrip(ec_ctx);
  printf("done\n");

  printf("Setting slaves in operational state... ");
  ec_ctx.write_state(0, EC_STATE_OPERATIONAL);
  /* Poll the result ten times before giving up */
  for (int i = 0; i < 10; ++i) {
    printf(".");
    fieldbus_roundtrip(ec_ctx);
    if (ec_ctx.slave_state(0) == EC_STATE_OPERATIONAL) {
      printf(" all slaves are now operational tries : %d \n", i);
      return true;
    }
  }

  // printf(" failed,");
  // ecx_readstate(context);
  // for (int i = 1; i <= fieldbus->slavecount; ++i) {
  //   slave = fieldbus->slavelist.data() + i;
  //   if (slave->state != EC_STATE_OPERATIONAL) {
  //     printf(" slave %d is 0x%04X (AL-status=0x%04X %s)", i, slave->state, slave->ALstatuscode,
  //            ec_ALstatuscode2string(slave->ALstatuscode));
  //   }
  // }
  // printf("\n");

  return false;
}

auto fieldbus_dump(tfc::ec::context_t& ctx) -> bool {
  return fieldbus_roundtrip(ctx) > 0;
}
// void fieldbus_check_state(tfc::ec::context& ec_ctx) {
//   //ecx_contextt* context;
//   //ec_groupt* grp;
//   //ec_slavet* slave;
//
//   //grp = context->grouplist + fieldbus->group;
//   //grp->docheckstate = FALSE;
//   ecx_readstate(context);
//   for (size_t i = 1; i <= ec_ctx.slave_count(); ++i) {
//     slave = context->slavelist + i;
//     if (slave->group != fieldbus->group) {
//       /* This slave is part of another group: do nothing */
//     } else if (slave->state != EC_STATE_OPERATIONAL) {
//       grp->docheckstate = TRUE;
//       if (slave->state == EC_STATE_SAFE_OP + EC_STATE_ERROR) {
//         printf("* Slave %zu is in SAFE_OP+ERROR, attempting ACK\n", i);
//         slave->state = EC_STATE_SAFE_OP + EC_STATE_ACK;
//         ecx_writestate(context, i);
//       } else if (slave->state == EC_STATE_SAFE_OP) {
//         printf("* Slave %zu is in SAFE_OP, change to OPERATIONAL\n", i);
//         slave->state = EC_STATE_OPERATIONAL;
//         ecx_writestate(context, i);
//       } else if (slave->state > EC_STATE_NONE) {
//         if (ecx_reconfig_slave(context, i, EC_TIMEOUTRET) != 0) {
//           slave->islost = FALSE;
//           printf("* Slave %zu reconfigured\n", i);
//         }
//       } else if (slave->islost == 0) {
//         ecx_statecheck(context, i, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
//         if (slave->state == EC_STATE_NONE) {
//           slave->islost = TRUE;
//           printf("* Slave %zu lost\n", i);
//         }
//       }
//     } else if (slave->islost != 0) {
//       if (slave->state != EC_STATE_NONE) {
//         slave->islost = FALSE;
//         printf("* Slave %zu found\n", i);
//       } else if (ecx_recover_slave(context, i, EC_TIMEOUTRET) != 0) {
//         slave->islost = FALSE;
//         printf("* Slave %zu recovered\n", i);
//       }
//     }
//   }
//
//   if (grp->docheckstate == 0) {
//     printf("All slaves resumed OPERATIONAL\n");
//   }
// }

auto main(int argc, char* argv[]) -> int {
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

  tfc::ec::context_t ctx(argv[1]);

  if (fieldbus_start(ctx)) {
    for (int i = 1; i <= 100000; ++i) {
      // printf("Iteration %4d:", i);
      if (fieldbus_dump(ctx)) {
        //fieldbus_check_state(ctx);
      }
      osal_usleep(5000);
    }
  }

  return 0;
}