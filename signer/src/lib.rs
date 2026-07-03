#![no_std]
#![allow(unsafe_op_in_unsafe_fn)]
#![feature(str_as_str)]

extern crate alloc;

use core::alloc::Layout;
use core::ptr;
use core::ptr::slice_from_raw_parts;
use rlp::RlpStream;
use sha3::{Digest, Keccak256};
use k256::ecdsa::SigningKey;

unsafe extern "C"{
    fn malloc(size: usize) -> *mut u8;
    fn free(ptr: *mut u8);
    fn realloc(ptr: *mut u8, size: usize) -> *mut u8;
}

struct Allocator;
unsafe impl core::alloc::GlobalAlloc for Allocator{
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        malloc(layout.size())
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        free(ptr);
    }

    unsafe fn realloc(&self, ptr: *mut u8, _layout: Layout, new_size: usize) -> *mut u8 {
        realloc(ptr, new_size)
    }
}

#[global_allocator]
static ALLOCATOR: Allocator = Allocator;

macro_rules! parse {
    ($len:expr, $from:ident) => {
        unsafe{
            let part = (slice_from_raw_parts($from, $len)).as_ref().unwrap();
            $from = $from.add($len);

            part.try_into().unwrap()
        }
    };
}

#[repr(C)]
pub struct Signature {
    pub r: [u8; 32],
    pub s: [u8; 32],
    pub v: [u8; 64]
}

#[repr(C)]
struct Signer<'a> {
    pub nonce: &'a[u8; 8],
    pub gas_price: &'a[u8; 16],
    pub gas_limit: &'a[u8; 8],
    pub to: &'a[u8; 20],
    pub value: &'a[u8; 16],
    pub data: Option<&'a[u8; 178]>,
}

impl<'a> Signer<'a>{
    #[unsafe(no_mangle)]
    pub extern "C" fn sign_transaction(
        ptr: *const u8,
        private_key: &[u8; 32],
        chain_id: &[u8; 4]
    ) -> Signature{
        let mut p = ptr;

        let nonce = parse!(8, p);
        let gas_price = parse!(16, p);
        let gas_limit = parse!(8, p);
        let to = parse!(20, p);
        let value = parse!(16, p);

        let data = if unsafe { *p } != '0' as u8{
            Some(unsafe {
                slice_from_raw_parts(p, 178)
                    .as_ref()
                    .unwrap()
                    .try_into()
                    .unwrap()
            })
        } else {
            None
        };

        let mut stream = if let Some(_) = data{
            RlpStream::new_list(9)
        } else {
            RlpStream::new_list(8)
        };

        stream.append_raw(nonce, nonce.len());
        stream.append_raw(gas_price, gas_price.len());
        stream.append_raw(gas_limit, gas_limit.len());
        stream.append_raw(to, to.len());
        stream.append_raw(value, value.len());

        stream.append_raw(chain_id, 4);
        if let Some(d) = data{
            stream.append_raw(d, d.len());
        }

        let rlp_encoded = stream.out();

        let mut hasher = Keccak256::new();
        hasher.update(&rlp_encoded.as_slice());
        let tx_hash = hasher.finalize();

        let signing_key = SigningKey::from_bytes(
            private_key.into()
        ).unwrap();

        let (signature, recovery_id) = signing_key
            .sign_prehash_recoverable(&tx_hash)
            .unwrap();

        let signature_bytes = signature.to_bytes();
        let mut r = [0u8; 32];
        let mut s = [0u8; 32];
        r.copy_from_slice(&signature_bytes[0..32]);
        s.copy_from_slice(&signature_bytes[32..64]);

        let mut v = [0u8; 64];
        let v_tmp = (chain_id[0] * 2) as u32 + 35 + (recovery_id.to_byte() as u32);
        unsafe { ptr::copy_nonoverlapping(&v_tmp, v.as_mut_ptr() as *mut u32, 32); };

        Signature{ r, s, v }
    }
}

#[panic_handler]
unsafe fn panic_handler(_: &core::panic::PanicInfo) -> !{
    loop { core::arch::asm!("wfi") }
}