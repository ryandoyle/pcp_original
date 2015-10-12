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

  end

end
