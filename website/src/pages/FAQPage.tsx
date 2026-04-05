import { HeroSection } from '../components/sections/HeroSection';
import { ProductFAQ } from '../components/product/ProductFAQ';
import { ContactCTASection } from '../components/sections/ContactCTASection';
import { sfsvcConfig } from '../content/products';
import type { FAQItem } from '../types/content';

const generalFAQ: FAQItem[] = [
  {
    question: 'How does the 7-day free trial work?',
    answer: 'When you start a trial, you enter your payment details through Stripe\'s secure checkout. Your SFSVC license key is delivered immediately. If you cancel before day 7, your card is never charged. After day 7, your subscription begins automatically.',
  },
  {
    question: 'How does the license key work?',
    answer: 'Each license key is an Ed25519 cryptographically signed token issued by our licensing server. It is bound to your drone\'s hardware fingerprint on first activation. The SFSVC SDK validates the key locally — no internet connection required during flight (BVLOS compatible).',
  },
  {
    question: 'Can I use one license on multiple drones?',
    answer: 'Each license tier supports a specific number of drone activations: Starter (1), Growth (4), Group (6), Enterprise (15). If you need more, upgrade your plan or contact us for a custom arrangement.',
  },
  {
    question: 'What happens if my license expires?',
    answer: 'SFSVC will output a license expired message and refuse to initialise the pipeline until the subscription is renewed. Renewal is automatic if your subscription is active.',
  },
  {
    question: 'What is the minimum hardware requirement?',
    answer: 'Any Linux x86_64 or ARM64 system with: 2+ CPU cores, 2 GB RAM, Ubuntu 20.04 or newer. Recommended: Intel N100 mini PC for optimal sub-2ms performance at 4.2W power draw.',
  },
];

export function FAQPage() {
  return (
    <>
      <HeroSection
        tag="FAQ"
        title="Frequently Asked Questions"
        subtitle="Technical, licensing, and deployment questions about the NEPA platform and product modules."
      />

      <ProductFAQ items={[...generalFAQ, ...(sfsvcConfig.faqItems || [])]} />

      <ContactCTASection
        title="Question not answered?"
        subtitle="Contact our engineering team at support@aurasensehk.com for technical enquiries."
        primaryLabel="Contact Support"
        primaryHref="/contact"
        secondaryLabel="View Products"
        secondaryHref="/products"
      />
    </>
  );
}
