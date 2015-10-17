require 'pcp/pmapi'

describe PCP::PMAPI do

  describe 'constants' do
    [
      PCP::PMAPI::PM_SPACE_BYTE,
      PCP::PMAPI::PM_SPACE_KBYTE,
      PCP::PMAPI::PM_SPACE_MBYTE,
      PCP::PMAPI::PM_SPACE_GBYTE,
      PCP::PMAPI::PM_SPACE_TBYTE,
      PCP::PMAPI::PM_SPACE_PBYTE,
      PCP::PMAPI::PM_SPACE_EBYTE,

      PCP::PMAPI::PM_TIME_NSEC,
      PCP::PMAPI::PM_TIME_USEC,
      PCP::PMAPI::PM_TIME_MSEC,
      PCP::PMAPI::PM_TIME_SEC,
      PCP::PMAPI::PM_TIME_MIN,
      PCP::PMAPI::PM_TIME_HOUR,

      PCP::PMAPI::PM_CONTEXT_UNDEF,
      PCP::PMAPI::PM_CONTEXT_HOST,
      PCP::PMAPI::PM_CONTEXT_ARCHIVE,
      PCP::PMAPI::PM_CONTEXT_LOCAL,
      PCP::PMAPI::PM_CONTEXT_TYPEMASK,
      PCP::PMAPI::PM_CTXFLAG_SECURE,
      PCP::PMAPI::PM_CTXFLAG_COMPRESS,
      PCP::PMAPI::PM_CTXFLAG_RELAXED,
      PCP::PMAPI::PM_CTXFLAG_AUTH,
      PCP::PMAPI::PM_CTXFLAG_CONTAINER,

      PCP::PMAPI::PMNS_LOCAL,
      PCP::PMAPI::PMNS_REMOTE,
      PCP::PMAPI::PMNS_ARCHIVE,

      PCP::PMAPI::PM_ERR_GENERIC,
      PCP::PMAPI::PM_ERR_PMNS,
      PCP::PMAPI::PM_ERR_NOPMNS,
      PCP::PMAPI::PM_ERR_DUPPMNS,
      PCP::PMAPI::PM_ERR_TEXT,
      PCP::PMAPI::PM_ERR_APPVERSION,
      PCP::PMAPI::PM_ERR_VALUE,
      PCP::PMAPI::PM_ERR_TIMEOUT,
      PCP::PMAPI::PM_ERR_NODATA,
      PCP::PMAPI::PM_ERR_RESET,
      PCP::PMAPI::PM_ERR_NAME,
      PCP::PMAPI::PM_ERR_PMID,
      PCP::PMAPI::PM_ERR_INDOM,
      PCP::PMAPI::PM_ERR_INST,
      PCP::PMAPI::PM_ERR_UNIT,
      PCP::PMAPI::PM_ERR_CONV,
      PCP::PMAPI::PM_ERR_TRUNC,
      PCP::PMAPI::PM_ERR_SIGN,
      PCP::PMAPI::PM_ERR_PROFILE,
      PCP::PMAPI::PM_ERR_IPC,
      PCP::PMAPI::PM_ERR_EOF,
      PCP::PMAPI::PM_ERR_NOTHOST,
      PCP::PMAPI::PM_ERR_EOL,
      PCP::PMAPI::PM_ERR_MODE,
      PCP::PMAPI::PM_ERR_LABEL,
      PCP::PMAPI::PM_ERR_LOGREC,
      PCP::PMAPI::PM_ERR_NOTARCHIVE,
      PCP::PMAPI::PM_ERR_LOGFILE,
      PCP::PMAPI::PM_ERR_NOCONTEXT,
      PCP::PMAPI::PM_ERR_PROFILESPEC,
      PCP::PMAPI::PM_ERR_PMID_LOG,
      PCP::PMAPI::PM_ERR_INDOM_LOG,
      PCP::PMAPI::PM_ERR_INST_LOG,
      PCP::PMAPI::PM_ERR_NOPROFILE,
      PCP::PMAPI::PM_ERR_NOAGENT,
      PCP::PMAPI::PM_ERR_PERMISSION,
      PCP::PMAPI::PM_ERR_CONNLIMIT,
      PCP::PMAPI::PM_ERR_AGAIN,
      PCP::PMAPI::PM_ERR_ISCONN,
      PCP::PMAPI::PM_ERR_NOTCONN,
      PCP::PMAPI::PM_ERR_NEEDPORT,
      PCP::PMAPI::PM_ERR_NONLEAF,
      PCP::PMAPI::PM_ERR_TYPE,
      PCP::PMAPI::PM_ERR_THREAD,
      PCP::PMAPI::PM_ERR_NOCONTAINER,
      PCP::PMAPI::PM_ERR_BADSTORE,
      PCP::PMAPI::PM_ERR_TOOSMALL,
      PCP::PMAPI::PM_ERR_TOOBIG,
      PCP::PMAPI::PM_ERR_FAULT,
      PCP::PMAPI::PM_ERR_PMDAREADY,
      PCP::PMAPI::PM_ERR_PMDANOTREADY,
      PCP::PMAPI::PM_ERR_NYI,

      PCP::PMAPI::PMNS_LEAF_STATUS,
      PCP::PMAPI::PMNS_NONLEAF_STATUS,
    ].each do |constant|
      it "should be numeric" do
        expect(constant).to be_a_kind_of Integer
      end
    end
  end

  describe 'integration tests' do

    let(:pmapi) { PCP::PMAPI.new(PCP::PMAPI::PM_CONTEXT_HOST, "localhost") }


    describe '#new' do
      it 'should construct without errors' do
        PCP::PMAPI.new(PCP::PMAPI::PM_CONTEXT_HOST, "localhost")
      end
    end

    describe '#pmGetContextHostName_r' do
      it 'should return the hostname of the created context' do
        expect(pmapi.pmGetContextHostName_r).to eq 'ryandesktop'
      end
      it 'should be have an alias from pmGetContextHostName' do
        expect(pmapi.pmGetContextHostName).to eq 'ryandesktop'
      end
    end

    describe '#pmGetPMNSLocation' do
      it 'returns the PMNS location' do
        expect(pmapi.pmGetPMNSLocation).to eq PCP::PMAPI::PMNS_REMOTE
      end
    end

    describe '#pmLoadNameSpace' do
      it 'should raise an error for non-existent files' do
        expect{pmapi.pmLoadNameSpace('some-namespace-that-doesnt-exist.txt')}.to raise_error PCP::PMAPI::NamespaceLoadError
      end
    end

    describe '#pmUnloadNameSpace' do
      it 'should raise an error for non-existent files' do
        expect{pmapi.pmUnloadNameSpace}.to_not raise_error
      end
    end

    describe '#pmTrimNameSpace' do
      it 'should not raise an error' do
        expect{pmapi.pmTrimNameSpace}.to_not raise_error
      end
    end

    describe '#pmLookupName' do
      it 'should fetch a single metric PMID' do
        expect(pmapi.pmLookupName(['disk.all.read'])).to eq [{"disk.all.read"=>251658264}]
      end
      it 'should fetch multiple metric PMIDs' do
        expect(pmapi.pmLookupName(['disk.all.read', 'disk.all.write'])).to eq [{"disk.all.read"=>251658264}, {"disk.all.write"=>251658265}]
      end
      it 'should raise an error for unknown names' do
        expect{pmapi.pmLookupName(['something.that.doesnt.exist'])}.to raise_error PCP::PMAPI::Error
      end
    end

    describe '#pmGetChildren' do
      it 'should return immediate children of the metric' do
        expect(pmapi.pmGetChildren('swap')).to eq ["pagesin", "pagesout", "in", "out", "free", "length", "used"]
      end
      it 'should raise an error for invalid metrics' do
        expect{pmapi.pmGetChildren('invalid.metric.name')}.to raise_error PCP::PMAPI::Error
      end
      it 'should return an empty list for metrics that are leaf nodes' do
        expect(pmapi.pmGetChildren('swap.used')).to eq []
      end
    end

    describe '#pmGetChildrenStatus' do
      it 'should return the children and their status type' do
        expect(pmapi.pmGetChildrenStatus('swap')).to eq [{"pagesin"=>0}, {"pagesout"=>0}, {"in"=>0}, {"out"=>0}, {"free"=>0}, {"length"=>0}, {"used"=>0}]
      end
      it 'should return children with a status flag indicating they are non-leaf nodes' do
        expect(pmapi.pmGetChildrenStatus('disk')).to eq [{"dev"=>1}, {"all"=>1}, {"partitions"=>1}, {"dm"=>1}]
      end
      it 'should return an empty list for metrics that are leaf nodes' do
        expect(pmapi.pmGetChildrenStatus('swap.used')).to eq []
      end
      it 'should raise an error for invalid metrics' do
        expect{pmapi.pmGetChildrenStatus('invalid.metric.name')}.to raise_error PCP::PMAPI::Error
      end
    end

    describe '#pmNameID' do
      it 'returns the name for a pmid' do
        expect(pmapi.pmNameID(251658264)).to eq 'disk.all.read'
      end
      it 'raises an error for invalid PMIDs' do
        expect{pmapi.pmNameID(123456)}.to raise_error PCP::PMAPI::InvalidPMIDError
      end
    end

    describe '#pmNameAll' do
      it 'returns all the names for a PMID' do
        expect(pmapi.pmNameAll(121634817)).to eq ["sample.dupnames.pid_daemon", "sample.dupnames.daemon_pid", "sample.daemon_pid"]
      end
      it 'raises an error for invalid PMIDs' do
        expect{pmapi.pmNameAll(123456)}.to raise_error PCP::PMAPI::InvalidPMIDError
      end
    end

    describe '#pmTraversePMNS' do
      it 'yields when traversing the PMNS namespace' do
        expect { |b| pmapi.pmTraversePMNS('swap', &b) }.to yield_successive_args("swap.pagesin", "swap.pagesout", "swap.in", "swap.out", "swap.free", "swap.length", "swap.used")
      end
      it 'raises an error for invalid metric IDs' do
        expect{ pmapi.pmTraversePMNS('some.invalid.name') }.to raise_error PCP::PMAPI::Error
      end
    end

    describe "#pmLookupDesc" do
      it 'returns the descriptor for a PMID' do
        expect(pmapi.pmLookupDesc(251658264)).to eq :pmid=>251658264, :type=>3, :indom=>4294967295, :sem=>1, :units=>{:dimSpace=>0, :dimTime=>0, :dimCount=>1, :scaleSpace=>0, :scaleTime=>0, :scaleCount=>0}
      end
      it 'raises an error for invalid metrics IDs' do
        expect{pmapi.pmLookupDesc(123456)}.to raise_error PCP::PMAPI::Error
      end
    end

    # sample.many.int == PMID: 121634896 == Instance domain 121634824
    describe '#pmLookupInDom' do
      it 'returns the internal instance identifier given an instance domain and the instance name' do
        expect(pmapi.pmLookupInDom(121634824, "i-0")).to eq 0
      end
      it 'returns an error for unknown instance identifiers' do
        expect{pmapi.pmLookupInDom(121634824, "doesntexist")}.to raise_error PCP::PMAPI::Error
      end
    end

    describe '#pmLookupInDomArchive' do
      it 'requires integration testing'
      it 'raises an error if not used in an archive context' do
        expect{pmapi.pmLookupInDomArchive(121634824, "i-0")}.to raise_error PCP::PMAPI::Error
      end
    end

    describe '#pmNameInDom' do
      it 'should return the text name of an instance from an instance domain and and instance id' do
        expect(pmapi.pmNameInDom(121634824, 0)).to eq "i-0"
      end
      it 'should raise an error for invalid instance domains' do
        expect{pmapi.pmNameInDom(123, 1)}.to raise_error PCP::PMAPI::Error
      end
      it 'should raise an error for invalid instance ids' do
        expect{pmapi.pmNameInDom(121634824, 123)}.to raise_error PCP::PMAPI::Error
      end
    end

    describe '#pmNameInDomArchive' do
      it 'requires integration testing'
      it 'raises an error if not used in an archive context' do
        expect{pmapi.pmNameInDomArchive(121634824, 0)}.to raise_error PCP::PMAPI::Error
      end
    end

  end

end
