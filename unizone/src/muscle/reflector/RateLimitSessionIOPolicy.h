/* This file is Copyright 2002 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef RateLimitSessionIOPolicy_h
#define RateLimitSessionIOPolicy_h

#include "reflector/AbstractSessionIOPolicy.h"

namespace muscle {

/** 
 * This policy allows you to enforce an aggregate maximum bandwidth usage for the set
 * of AbstractReflectSessionSessions that use it.  Each Policy
 * 
 */
class RateLimitSessionIOPolicy : public AbstractSessionIOPolicy
{
public:
   /** Constructor.  
     * @param maxRate The maximum aggregate transfer rate to be enforced for all sessions
     *                that use this policy, in bytes per second.
     * @param primeBytes When the bytes first start to flow, the policy allows the first (primeBytes)
     *                   bytes to be sent out immediately, before clamping down on the flow rate.
     *                   This helps keep the policy from having to wake up the server too often,
     *                   and saves CPU time.  This parameter lets you adjust that startup-size.
     *                   Defaults to 2048 bytes.
     */
   RateLimitSessionIOPolicy(uint32 maxRate, uint32 primeBytes = 2048);

   /** Destructor. */
   virtual ~RateLimitSessionIOPolicy();

   virtual void PolicyHolderAdded(const PolicyHolder & ph);
   virtual void PolicyHolderRemoved(const PolicyHolder & ph);

   virtual void BeginIO(uint64 now);
   virtual bool OkayToTransfer(const PolicyHolder & ph);
   virtual uint32 GetMaxTransferChunkSize(const PolicyHolder & ph);
   virtual void BytesTransferred(const PolicyHolder & ph, uint32 numBytes);
   virtual void EndIO(uint64 now);

   virtual uint64 GetPulseTime(uint64 now, uint64 prevResult);
   virtual void Pulse(uint64 now, uint64 schedTime);

private:
   void UpdateTransferTally(uint64 now);

   uint32 _maxRate;
   uint32 _byteLimit;
   uint64 _lastTransferAt;
   uint32 _transferTally;
   uint32 _numParticipants;
};

};  // end namespace muscle

#endif
