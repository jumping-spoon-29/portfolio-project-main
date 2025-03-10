#include <cstdint>

#include <tuple>

#include <vector>

// #include ... other project headers

namespace eagle::dasm
{
	struct basic_block
	{
		uint32_t rva_begin, uint32_t rva_end;
		uint32_t branch_one, branch_two;

		std::vector<codec::dec::inst> insts;
	}

	class dasm_kernel
	{
	protected:
		/// @brief decodes an instruction at the current rva, the function assumes that the rva is located at a valid instruction
		/// @return pair with [decoded instruction, instruction length] at the current rva
		virtual std::pair<codec::dec::inst, uint8_t> decode_current() = 0;

		/// @brief decodes the instruction at the current rva and returns branches
		/// @return returns a list of rvas the instruction branches to. len(0) if none, len(1) if jmp, len(2) if conditional jump
		virtual std::vector<uint32_t> get_branches() = 0;

		/// @brief getter for the current rva
		/// @return the current rva
		virtual uint32_t get_current_rva();

		/// @brief updates the current rva
		/// @param rva new rva
		/// @return old rva before replacement
		virtual uint32_t set_current_rva(uint32_t rva);
	};

	class segment_dasm : private dasm_kernel
	{
	public:
		// some kind of ctor for file data here
		// ...

		/// @brief dissasembled instructions until a branching instruction is reached at the current block
		/// @param rva the rva at which the target block begins
		/// @return the basic block the instructions create
		basic_block get_block(uint32_t rva)
		{
			set_current_rva(rva);

			basic_block block{};
			block.rva_begin = get_current_rva();

			while (does_branch() == 0)
			{
				auto [result, size] = decode_current();
				block.insts.push_back(result);

				block.rva_end += size;
				set_current_rva(block.rva_end);
			}

			auto branches = get_branches();
			block.branch_one = std::get<0>(branches);
			block.branch_two = std::get<1>(branches);

			return block;
		}

		/// @brief gets all the instructions in a certain section and disregards the flow of the instructions
		/// @param rva_begin the rva at which the starting instruction is at
		/// @param rva_end the inclusive rva at which the last instruction ends
		/// @return the list of instructions which are contained within this range
		std::vector<codec::dec::inst> dump_section(uint32_t rva_begin, uint32_t rva_end)
		{
			std::vector<codec::dec::inst> insts;

			uint32_t rva_current = rva_begin;
			for (int i = rva_current; i < rva_end;)
			{
				auto [result, size] = decode_current();
				insts.push_back(result);

				rva_current += size;
				set_current_rva(block.rva_end);
			}

			return insts;
		}

	private:
		uint32_t rva_begin;
		uint32_t rva_end;

		// std::pair<codec::dec::inst, uint8_t> decode_current() override{ }
		// std::vector<uint32_t> get_branches() override { }

		/// @brief getter for the current rva
		/// @return the current rva
		virtual uint32_t get_current_rva() override {};

		/// @brief updates the current rva
		/// @param rva new rva
		/// @return old rva before replacement
		virtual uint32_t set_current_rva(uint32_t rva) override {};
	};
}

int main()
{
	// some main function which loads instructions
	// ...

	std::vec<uint8_t> bin_data = ...;
	eagle::dasm::segment_dasm dasm(bin_data);

	std::vector<codec::dec::inst> insts = dasm.dump_section();
	for (auto inst : insts)
		print(inst); // dump all the instructions for the entire section into a print

	std::set<uint32_t> discovered_rvas;
	std::vector<basic_block> blocks;

	std::dequeue<uint32> rva_queue;
	rva_queue.push(start_rva);

	while (rva_queue.empty())
	{
		eagle::dasm::segment_dasm dasm(bin_data);
		basic_block block = dasm.get_block();

		auto insert_branch = [&](auto branch_rva)
		{
			if (branch_rva != -1)
			{
				if (discovered_rvas.insert(branch_rva))
					rva_queue.push(branch_rva);
			}
		};

		insert_branch(block.branch_one);
		insert_branch(block.branch_two);

		blocks.push_back(block);
	}

	print("here are the discovered blocks");
	for (auto &block : blocks)
	{
		print("block begins: " + block.rva_begin + " block ends: " + block.rva_end);
		for (auto inst : block.insts)
			print(inst);
	}
}