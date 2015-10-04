require 'pcp/pmapi'

describe PCP::PMAPI do

  describe 'space constants' do
    it 'should contain the constant for PM_SPACE_BYTE' do
      expect(PCP::PMAPI::PM_SPACE_BYTE).to eq 0
    end

    it 'should contain the constant for PM_SPACE_KBYTE' do
      expect(PCP::PMAPI::PM_SPACE_KBYTE).to eq 1
    end

    it 'should contain the constant for PM_SPACE_MBYTE' do
      expect(PCP::PMAPI::PM_SPACE_MBYTE).to eq 2
    end

    it 'should contain the constant for PM_SPACE_GBYTE' do
      expect(PCP::PMAPI::PM_SPACE_GBYTE).to eq 3
    end

    it 'should contain the constant for PM_SPACE_TBYTE' do
      expect(PCP::PMAPI::PM_SPACE_TBYTE).to eq 4
    end

    it 'should contain the constant for PM_SPACE_PBYTE' do
      expect(PCP::PMAPI::PM_SPACE_PBYTE).to eq 5
    end

    it 'should contain the constant for PM_SPACE_EBYTE' do
      expect(PCP::PMAPI::PM_SPACE_EBYTE).to eq 6
    end
  end

  describe 'context constants' do
    it 'should contain the constant for PM_CONTEXT_UNDEF' do
      expect(PCP::PMAPI::PM_CONTEXT_UNDEF).to eq -1
    end
    it 'should contain the constant for PM_CONTEXT_HOST' do
      expect(PCP::PMAPI::PM_CONTEXT_HOST).to eq 1
    end
    it 'should contain the constant for PM_CONTEXT_ARCHIVE' do
      expect(PCP::PMAPI::PM_CONTEXT_ARCHIVE).to eq 2
    end
    it 'should contain the constant for PM_CONTEXT_LOCAL' do
      expect(PCP::PMAPI::PM_CONTEXT_LOCAL).to eq 3
    end
    it 'should contain the constant for PM_CONTEXT_TYPEMASK' do
      expect(PCP::PMAPI::PM_CONTEXT_TYPEMASK).to eq 255
    end
    it 'should contain the constant for PM_CTXFLAG_SECURE' do
      expect(PCP::PMAPI::PM_CTXFLAG_SECURE).to eq 1024
    end
    it 'should contain the constant for PM_CTXFLAG_COMPRESS' do
      expect(PCP::PMAPI::PM_CTXFLAG_COMPRESS).to eq 2048
    end
    it 'should contain the constant for PM_CTXFLAG_RELAXED' do
      expect(PCP::PMAPI::PM_CTXFLAG_RELAXED).to eq 4096
    end
    it 'should contain the constant for PM_CTXFLAG_AUTH' do
      expect(PCP::PMAPI::PM_CTXFLAG_AUTH).to eq 8192
    end
    it 'should contain the constant for PM_CTXFLAG_CONTAINER' do
      expect(PCP::PMAPI::PM_CTXFLAG_CONTAINER).to eq 16384
    end
  end

  describe '#new' do
    it 'should construct without errors' do
      PCP::PMAPI.new(PCP::PMAPI::PM_CONTEXT_HOST, "localhost")
    end
  end

end
