require 'pcp_native'

module PCP
  class PMAPI

    def initialize(metric_source, metric_source_argument)
      unless(metric_source_argument.class == String || metric_source_argument == nil)
        raise TypeError, 'Metric source argument should be String or Nil' unless metric_source_argument
      end
      pmNewContext(metric_source, metric_source_argument)
    end

    alias_method :pmGetContextHostName, :pmGetContextHostName_r

  end
end
