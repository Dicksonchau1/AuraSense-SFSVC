import { SectionWrapper } from '../components/ui/SectionWrapper';
export function FAQPage() {
  return (<main><SectionWrapper compact><span className="module-label">FAQ</span><h1 style={{ fontSize:'clamp(2rem,4vw,3rem)', fontWeight:700, lineHeight:1.15, letterSpacing:'-0.03em', color:'var(--color-text-primary)' }}>Frequently Asked Questions</h1></SectionWrapper></main>);
}
