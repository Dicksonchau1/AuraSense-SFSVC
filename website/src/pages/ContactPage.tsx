import { HeroSection } from '../components/sections/HeroSection';
import { PageContainer } from '../components/layout/PageContainer';
import { SurfaceCard } from '../components/ui/SurfaceCard';
import { CTAButton } from '../components/ui/CTAButton';

export function ContactPage() {
  return (
    <>
      <HeroSection
        tag="Contact"
        title="Contact Engineering"
        subtitle="For technical evaluation, deployment planning, SDK access, or procurement documentation."
      />

      <section className="py-20">
        <PageContainer>
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-12">
            <div>
              <h2 className="text-xl font-semibold text-text-primary mb-6">Get in touch</h2>
              <div className="space-y-4">
                <SurfaceCard>
                  <h3 className="text-sm font-medium text-text-primary mb-1">Engineering & SDK Access</h3>
                  <p className="text-sm text-text-secondary">support@aurasensehk.com</p>
                </SurfaceCard>
                <SurfaceCard>
                  <h3 className="text-sm font-medium text-text-primary mb-1">Enterprise & Procurement</h3>
                  <p className="text-sm text-text-secondary">support@aurasensehk.com</p>
                </SurfaceCard>
                <SurfaceCard>
                  <h3 className="text-sm font-medium text-text-primary mb-1">Location</h3>
                  <p className="text-sm text-text-secondary">Hong Kong SAR</p>
                </SurfaceCard>
              </div>
            </div>

            <div>
              <h2 className="text-xl font-semibold text-text-primary mb-6">Send a message</h2>
              {/* TODO: Connect to actual form handler / API */}
              <form className="space-y-4" onSubmit={(e) => e.preventDefault()}>
                <div>
                  <label htmlFor="name" className="block text-sm text-text-secondary mb-1.5">Name</label>
                  <input
                    type="text"
                    id="name"
                    className="w-full bg-bg-surface border border-border-default rounded-sm px-4 py-2.5 text-sm text-text-primary placeholder:text-text-muted focus:outline-none focus:border-accent-primary/50 transition-colors"
                    placeholder="Your name"
                  />
                </div>
                <div>
                  <label htmlFor="email" className="block text-sm text-text-secondary mb-1.5">Email</label>
                  <input
                    type="email"
                    id="email"
                    className="w-full bg-bg-surface border border-border-default rounded-sm px-4 py-2.5 text-sm text-text-primary placeholder:text-text-muted focus:outline-none focus:border-accent-primary/50 transition-colors"
                    placeholder="you@company.com"
                  />
                </div>
                <div>
                  <label htmlFor="subject" className="block text-sm text-text-secondary mb-1.5">Subject</label>
                  <select
                    id="subject"
                    className="w-full bg-bg-surface border border-border-default rounded-sm px-4 py-2.5 text-sm text-text-primary focus:outline-none focus:border-accent-primary/50 transition-colors"
                  >
                    <option value="">Select a topic</option>
                    <option value="sdk">SDK Access</option>
                    <option value="enterprise">Enterprise Enquiry</option>
                    <option value="technical">Technical Question</option>
                    <option value="procurement">Procurement</option>
                    <option value="other">Other</option>
                  </select>
                </div>
                <div>
                  <label htmlFor="message" className="block text-sm text-text-secondary mb-1.5">Message</label>
                  <textarea
                    id="message"
                    rows={5}
                    className="w-full bg-bg-surface border border-border-default rounded-sm px-4 py-2.5 text-sm text-text-primary placeholder:text-text-muted focus:outline-none focus:border-accent-primary/50 transition-colors resize-y"
                    placeholder="Describe your requirements..."
                  />
                </div>
                <CTAButton label="Send Message" href="#" variant="primary" size="md" />
              </form>
            </div>
          </div>
        </PageContainer>
      </section>
    </>
  );
}
