#include "artdaq/Application/Routing/RoutingMasterPolicy.hh"
#include "artdaq/Application/Routing/PolicyMacros.hh"
#include <fhiclcpp/ParameterSet.h>
#include <messagefacility/MessageLogger/MessageLogger.h>

namespace demo
{
	class MisbehaviorTest : public artdaq::RoutingMasterPolicy
	{
	public:
		explicit MisbehaviorTest(fhicl::ParameterSet ps);

		virtual ~MisbehaviorTest() { }

		artdaq::detail::RoutingPacket GetCurrentTable() override;
	private:
		artdaq::Fragment::sequence_id_t misbehave_after_;
		size_t misbehave_pause_ms_;
		bool misbehave_conflicting_table_data_;
		bool misbehave_corrupt_table_data_;
		bool misbehave_overload_event_builder_;
	};

	MisbehaviorTest::MisbehaviorTest(fhicl::ParameterSet ps)
		: RoutingMasterPolicy(ps)
		, misbehave_after_(ps.get<size_t>("misbehave_after_n_events", 1000))
		, misbehave_pause_ms_(ps.get<size_t>("misbehave_pause_time_ms", 0))
		, misbehave_conflicting_table_data_(ps.get<bool>("misbehave_send_conflicting_table_data", false))
		, misbehave_corrupt_table_data_(ps.get<bool>("misbehave_send_corrupt_table_data", false))
		, misbehave_overload_event_builder_(ps.get<bool>("misbehave_overload_event_builder", false))
	{
		auto count = (misbehave_conflicting_table_data_ ? 1 : 0) + (misbehave_corrupt_table_data_ ? 1 : 0) + (misbehave_overload_event_builder_ ? 1 : 0) + (misbehave_pause_ms_ > 0 ? 1 : 0);
		if (count > 1)
		{
			mf::LogWarning("MisbehaviorTest") << "Only one misbehavior is allowed at a time!";
			exit(3);
		}
	}

	artdaq::detail::RoutingPacket MisbehaviorTest::GetCurrentTable()
	{
		auto tokens = getTokensSnapshot();
		artdaq::detail::RoutingPacket output;

		auto half = tokens->size() / 2;
		size_t counter = 0;
		for(;counter < half;++counter)
		{
			output.emplace_back(artdaq::detail::RoutingPacketEntry(next_sequence_id_++, tokens->at(counter)));
		}

		if (next_sequence_id_ > misbehave_after_)
		{
			if (tokens->size() > 0)
			{
				if (misbehave_pause_ms_ > 0)
				{
					mf::LogError("MisbehaviorTest") << "Pausing for " << misbehave_pause_ms_ << " milliseconds before sending table update";
					usleep(misbehave_pause_ms_ * 1000);
				}
				if (misbehave_conflicting_table_data_)
				{
					mf::LogError("MisbehaviorTest") << "Adding conflicting data point to output";
					output.emplace_back(next_sequence_id_, tokens->at(counter) + 1);
				}
				if (misbehave_corrupt_table_data_)
				{
					mf::LogError("MisbehaviorTest") << "Adding random data point";
					output.emplace_back(rand(), rand());
				}
				if (misbehave_overload_event_builder_)
				{
					mf::LogError("MisbehaviorTest") << "Sending 100 events in a row to Rank " << tokens->at(0);
					for (auto ii = 0; ii < 100; ++ii)
					{
						output.emplace_back(next_sequence_id_++, tokens->at(0));
					}
				}
				misbehave_after_ += misbehave_after_;
			}
		}

		for (;counter < tokens->size();++counter)
		{
			output.emplace_back(artdaq::detail::RoutingPacketEntry(next_sequence_id_++, tokens->at(counter)));
		}

		return output;
	}
}

DEFINE_ARTDAQ_ROUTING_POLICY(demo::MisbehaviorTest)