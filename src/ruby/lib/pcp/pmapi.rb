require 'pcp_native'

module PCP
  class PMAPI

    def initialize(metric_source, metric_source_argument)
      unless(metric_source_argument.class == String || metric_source_argument == nil)
        raise TypeError, 'Metric source argument should be String or Nil' unless metric_source_argument
      end
      native_pm_new_context(metric_source, metric_source_argument)
    end

  end
end
