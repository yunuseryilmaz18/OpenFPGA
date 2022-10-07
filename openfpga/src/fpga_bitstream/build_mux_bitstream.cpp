/********************************************************************
 * This file includes functions to build bitstream from routing multiplexers
 * which are based on different technology
 *******************************************************************/
/* Headers from vtrutil library */
#include "vtr_assert.h"
#include "vtr_log.h"
#include "vtr_vector.h"

/* Headers from openfpgautil library */
#include "build_mux_bitstream.h"
#include "decoder_library_utils.h"
#include "mux_bitstream_constants.h"
#include "mux_utils.h"
#include "openfpga_decode.h"

/* begin namespace openfpga */
namespace openfpga {

/********************************************************************
 * Find the default path id of a MUX
 * This is applied when the path id specified is DEFAULT_PATH_ID,
 * which is not correlated to the MUX implementation
 * This function is binding the default path id to the implemented structure
 * 1. If the MUX has a constant input, the default path id will be
 *    directed to the last input of the MUX, which is the constant input
 * 2. If the MUX does not have a constant input, the default path id
 *    will the first input of the MUX.
 *
 * Restriction:
 *   we assume the default path is the first input of the MUX
 *   Change if this is not what you want
 *******************************************************************/
size_t find_mux_default_path_id(const CircuitLibrary& circuit_lib,
                                const CircuitModelId& mux_model,
                                const size_t& mux_size) {
  size_t default_path_id;

  if (true == circuit_lib.mux_add_const_input(mux_model)) {
    default_path_id =
      mux_size - 1; /* When there is a constant input, use the last path */
  } else {
    default_path_id = DEFAULT_MUX_PATH_ID; /* When there is no constant input,
                                              use the default one */
  }

  return default_path_id;
}

/********************************************************************
 * This function generates bitstream for a CMOS routing multiplexer
 * Thanks to MuxGraph object has already describe the internal multiplexing
 * structure, bitstream generation is simply done by routing the signal
 * to from a given input to the output
 * All the memory bits can be generated by an API of MuxGraph
 *
 * To be generic, this function only returns a vector bit values
 * without touching an bitstream-relate data structure
 *******************************************************************/
static std::vector<bool> build_cmos_mux_bitstream(
  const CircuitLibrary& circuit_lib, const CircuitModelId& mux_model,
  const MuxLibrary& mux_lib, const size_t& mux_size, const int& path_id) {
  /* Note that the size of implemented mux could be different than the mux size
   * we see here, due to the constant inputs We will find the input size of
   * implemented MUX and fetch the graph-based representation in MUX library
   */
  size_t implemented_mux_size =
    find_mux_implementation_num_inputs(circuit_lib, mux_model, mux_size);
  /* Note that the mux graph is indexed using datapath MUX size!!!! */
  MuxId mux_graph_id = mux_lib.mux_graph(mux_model, mux_size);
  const MuxGraph mux_graph = mux_lib.mux_graph(mux_graph_id);

  size_t datapath_id = path_id;

  /* Find the path_id related to the implementation */
  if (DEFAULT_PATH_ID == path_id) {
    datapath_id =
      find_mux_default_path_id(circuit_lib, mux_model, implemented_mux_size);
  } else {
    VTR_ASSERT(datapath_id < mux_size);
  }
  /* Path id should makes sense */
  VTR_ASSERT(datapath_id < mux_graph.inputs().size());
  /* We should have only one output for this MUX! */
  VTR_ASSERT(1 == mux_graph.outputs().size());

  /* Generate the memory bits */
  vtr::vector<MuxMemId, bool> raw_bitstream = mux_graph.decode_memory_bits(
    MuxInputId(datapath_id), mux_graph.output_id(mux_graph.outputs()[0]));

  std::vector<bool> mux_bitstream;
  for (const bool& bit : raw_bitstream) {
    mux_bitstream.push_back(bit);
  }

  /* Consider local encoder support, we need further encode the bitstream */
  if (false == circuit_lib.mux_use_local_encoder(mux_model)) {
    return mux_bitstream;
  }

  /* Clear the mux_bitstream, we need to apply encoding */
  mux_bitstream.clear();

  /* Encode the memory bits level by level,
   * One local encoder is used for each level of multiplexers
   */
  for (const size_t& level : mux_graph.levels()) {
    /* The encoder will convert the path_id to a binary number
     * For example: when path_id=3 (use the 4th input), using a 2-input encoder
     * the sram_bits will be the 2-digit binary number of 3: 10
     */
    std::vector<size_t> encoder_data;

    /* Exception: there is only 1 memory at this level, bitstream will not be
     * changed!!! */
    if (1 == mux_graph.memories_at_level(level).size()) {
      mux_bitstream.push_back(
        raw_bitstream[mux_graph.memories_at_level(level)[0]]);
      continue;
    }

    /* Otherwise: we follow a regular recipe */
    for (size_t mem_index = 0;
         mem_index < mux_graph.memories_at_level(level).size(); ++mem_index) {
      /* Conversion rule: true = 1, false = 0 */
      if (true ==
          raw_bitstream[mux_graph.memories_at_level(level)[mem_index]]) {
        encoder_data.push_back(mem_index);
      }
    }
    /* There should be at most one '1' */
    VTR_ASSERT((0 == encoder_data.size()) || (1 == encoder_data.size()));
    /* Convert to encoded bits */
    std::vector<size_t> encoder_addr;
    if (0 == encoder_data.size()) {
      encoder_addr =
        itobin_vec(0, find_mux_local_decoder_addr_size(
                        mux_graph.memories_at_level(level).size()));
    } else {
      VTR_ASSERT(1 == encoder_data.size());
      encoder_addr = itobin_vec(encoder_data[0],
                                find_mux_local_decoder_addr_size(
                                  mux_graph.memories_at_level(level).size()));
    }
    /* Build final mux bitstream */
    for (const size_t& bit : encoder_addr) {
      mux_bitstream.push_back(1 == bit);
    }
  }

  return mux_bitstream;
}

/********************************************************************
 * This function generates bitstream for a routing multiplexer
 * supporting both CMOS and ReRAM multiplexer designs
 *******************************************************************/
std::vector<bool> build_mux_bitstream(const CircuitLibrary& circuit_lib,
                                      const CircuitModelId& mux_model,
                                      const MuxLibrary& mux_lib,
                                      const size_t& mux_size,
                                      const int& path_id) {
  std::vector<bool> mux_bitstream;

  switch (circuit_lib.design_tech_type(mux_model)) {
    case CIRCUIT_MODEL_DESIGN_CMOS:
      mux_bitstream = build_cmos_mux_bitstream(circuit_lib, mux_model, mux_lib,
                                               mux_size, path_id);
      break;
    case CIRCUIT_MODEL_DESIGN_RRAM:
      /* TODO: ReRAM MUX needs a different bitstream generation strategy */
      break;
    default:
      VTR_LOGF_ERROR(__FILE__, __LINE__,
                     "Invalid design technology for circuit model '%s'!\n",
                     circuit_lib.model_name(mux_model).c_str());
      exit(1);
  }
  return mux_bitstream;
}

} /* end namespace openfpga */
