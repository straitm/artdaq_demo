#include "artdaq/Application/Routing/RoutingMasterPolicy.hh"
#include "artdaq/Application/Routing/PolicyMacros.hh"
#include "fhiclcpp/ParameterSet.h"
#include "cetlib_except/exception.h"

namespace artdaq
{
	/**
	 * \brief An example RoutingMasterPolicy which redirects every Nth event to a desginated destination.
	 * Other events are Round-Robin'ed to the other configured destinations.
	 */
	class NthEventPolicy : public RoutingMasterPolicy
	{
	public:
		explicit NthEventPolicy(fhicl::ParameterSet ps);

		virtual ~NthEventPolicy() { }

		detail::RoutingPacket GetCurrentTable() override;
	private:
		size_t nth_;
		int nth_rank_;
	};

	/**
	 * \brief NthEventPolicy Constructor
	 * \param ps ParameterSet used to configure the NthEventPolicy
	 *
	 * \verbatim
	 * NthEventPolicy accepts the following Parameters:
	 * "nth_event" (REQUIRED): Every event where sequence_id % nth == 0 will be sent to
	 * "target_receiver" (REQUIRED): Recevier to which the nth_event will be sent
	 * \endverbatim
	 */
	NthEventPolicy::NthEventPolicy(fhicl::ParameterSet ps)
		: RoutingMasterPolicy(ps)
		, nth_(ps.get<size_t>("nth_event"))
		, nth_rank_(ps.get<int>("target_receiver"))
	{
		if (nth_ == 0) throw cet::exception("NthEvent_policy") << "nth_event must be greater than 0!";
	}

	/**
	 * \brief Construct a Routing Table using the current tokens
	 * \return A detail::RoutingPacket with the table. The table will contain full "turns" through the set of "regular" receivers, with
	 * the "nth" receiver inserted where sequence_id % nth == 0. If nth is mid-"turn" and no target_receiver tokens are availabe, it will
	 * not start the "turn".
	 */
	detail::RoutingPacket NthEventPolicy::GetCurrentTable()
	{
		auto tokens = getTokensSnapshot();
		std::map<int, int> table;
		for (auto token : *tokens.get())
		{
			table[token]++;
		}
		if (table.count(nth_rank_) == 0) table[nth_rank_] = 0;
		tokens->clear();

		detail::RoutingPacket output;
		TLOG_ARB(5, "NthEvent_policy") << "table[nth_rank_]=" << std::to_string(table[nth_rank_])
			<< ", Next nth=" << std::to_string(((next_sequence_id_ / nth_) + 1) * nth_)
			<< ", max seq=" << std::to_string(next_sequence_id_ + table.size() - 1) << TLOG_ENDL;
		auto endCondition = table.size() < GetReceiverCount() || (table[nth_rank_] <= 0 && (next_sequence_id_ % nth_ == 0 || ((next_sequence_id_ / nth_) + 1) * nth_ < next_sequence_id_ + table.size() - 1));
		while (!endCondition)
		{
			for (auto r : table)
			{
				TLOG_ARB(5, "NthEvent_policy") << "nth_=" << std::to_string(nth_)
					<< ", nth_rank=" << std::to_string(nth_rank_)
					<< ", r=" << std::to_string(r.first)
					<< ", next_sequence_id=" << std::to_string(next_sequence_id_) << TLOG_ENDL;
				if (next_sequence_id_ % nth_ == 0)
				{
					TLOG_ARB(5, "NthEvent_policy") << "Diverting event " << std::to_string(next_sequence_id_) << " to EVB " << nth_rank_ << TLOG_ENDL;
					output.emplace_back(detail::RoutingPacketEntry(next_sequence_id_++, nth_rank_));
					table[nth_rank_]--;
				}
				if (r.first != nth_rank_) {
					TLOG_ARB(5, "NthEvent_policy") << "Sending event " << std::to_string(next_sequence_id_) << " to EVB " << r.first << TLOG_ENDL;
					output.emplace_back(detail::RoutingPacketEntry(next_sequence_id_++, r.first));
					if (!endCondition) endCondition = r.second == 1;
					table[r.first]--;
				}
			}
			TLOG_ARB(5, "NthEvent_policy") << "table[nth_rank_]=" << std::to_string(table[nth_rank_])
				<< ", Next nth=" << std::to_string(((next_sequence_id_ / nth_) + 1) * nth_)
				<< ", max seq=" << std::to_string(next_sequence_id_ + table.size() - 1) << TLOG_ENDL;
			endCondition = endCondition || (table[nth_rank_] <= 0 && (next_sequence_id_ % nth_ == 0 || (next_sequence_id_ / nth_) * nth_ + nth_ < next_sequence_id_ + table.size() - 1));
		}

		for (auto r : table)
		{
			for (auto i = 0; i < r.second; ++i)
			{
				tokens->push_back(r.first);
			}
		}
		addUnusedTokens(std::move(tokens));

		return output;
	}
}

DEFINE_ARTDAQ_ROUTING_POLICY(artdaq::NthEventPolicy)
