import { useEffect, useState } from 'react';
import { Hero } from '../components/sections/Hero';
import { SectionWrapper } from '../components/ui/SectionWrapper';
import { Button } from '../components/ui/Button';
import {
  resourcesPageContent,
  resourceCategories,
  featuredResources,
  resourcesIntro,
  audienceSegments,
  accessModel,
  signInPanel,
  resourcesCta,
} from '../content/resources';
import type { ResourceEntry } from '../types/content';

export function ResourcesPage() {
  useEffect(() => {
    document.title = 'Resources — AuraSense';
  }, []);

  const [activeTag, setActiveTag] = useState<string | null>(null);

  const filtered: ResourceEntry[] = activeTag
    ? featuredResources.filter(
        (r) =>
          r.type.toLowerCase().includes(activeTag.toLowerCase()) ||
          r.audience.toLowerCase().includes(activeTag.toLowerCase()),
      )
    : featuredResources;

  return (
    <main>
      <Hero content={resourcesPageContent.hero} />

      {/* Intro */}
      <SectionWrapper>
        <div className="max-w-3xl mx-auto text-center">
          <h2 className="text-2xl font-semibold text-text-primary mb-4">
            {resourcesIntro.heading}
          </h2>
          <p className="text-text-secondary leading-relaxed">
            {resourcesIntro.body}
          </p>
        </div>
      </SectionWrapper>

      {/* Category tag strip */}
      <SectionWrapper>
        <div className="flex flex-wrap justify-center gap-2 mb-12">
          <button
            type="button"
            onClick={() => setActiveTag(null)}
            className={
              'px-4 py-1.5 rounded-full text-sm transition-colors ' +
              (activeTag === null
                ? 'bg-accent text-white'
                : 'bg-surface-overlay text-text-secondary hover:text-text-primary')
            }
          >
            All
          </button>
          {resourceCategories.map((cat) => (
            <button
              key={cat}
              type="button"
              onClick={() => setActiveTag(cat === activeTag ? null : cat)}
              className={
                'px-4 py-1.5 rounded-full text-sm transition-colors ' +
                (activeTag === cat
                  ? 'bg-accent text-white'
                  : 'bg-surface-overlay text-text-secondary hover:text-text-primary')
              }
            >
              {cat}
            </button>
          ))}
        </div>

        {/* Featured resources grid */}
        <div className="grid gap-6 sm:grid-cols-2 lg:grid-cols-3">
          {filtered.map((resource) => (
            <div
              key={resource.title}
              className="border border-border-subtle rounded-lg p-6 bg-surface-raised transition-all duration-200 hover:border-accent/40 hover:-translate-y-1 hover:shadow-lg hover:shadow-accent/5"
            >
              <div className="flex flex-wrap gap-2 mb-3">
                <span className="text-xs font-medium px-2 py-0.5 rounded-full bg-accent/10 text-accent">
                  {resource.type}
                </span>
                <span className="text-xs font-medium px-2 py-0.5 rounded-full bg-surface-overlay text-text-muted">
                  {resource.audience}
                </span>
              </div>
              <h3 className="text-lg font-semibold text-text-primary mb-2">
                {resource.title}
              </h3>
              <p className="text-sm text-text-secondary mb-4 leading-relaxed">
                {resource.description}
              </p>
              {resource.cta && (
                <Button variant="secondary" size="sm" href={resource.cta.href}>
                  {resource.cta.label}
                </Button>
              )}
            </div>
          ))}
        </div>
      </SectionWrapper>

      {/* Audience segments */}
      <SectionWrapper>
        <h2 className="text-2xl font-semibold text-text-primary text-center mb-8">
          Built for your role
        </h2>
        <div className="grid gap-6 sm:grid-cols-2 lg:grid-cols-4">
          {audienceSegments.map((seg) => (
            <div
              key={seg.title}
              className="border border-border-subtle rounded-lg p-6 bg-surface-raised transition-all duration-200 hover:border-accent/40 hover:-translate-y-1 hover:shadow-lg hover:shadow-accent/5"
            >
              <h3 className="text-lg font-semibold text-text-primary mb-2">
                {seg.title}
              </h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                {seg.description}
              </p>
            </div>
          ))}
        </div>
      </SectionWrapper>

      {/* Access model */}
      <SectionWrapper>
        <div className="max-w-3xl mx-auto text-center mb-8">
          <h2 className="text-2xl font-semibold text-text-primary mb-4">
            {accessModel.heading}
          </h2>
          <p className="text-text-secondary leading-relaxed">
            {accessModel.body}
          </p>
        </div>
        <div className="grid gap-6 sm:grid-cols-3 max-w-4xl mx-auto">
          {accessModel.items.map((item) => (
            <div
              key={item.title}
              className="border border-border-subtle rounded-lg p-6 bg-surface-raised transition-all duration-200 hover:border-accent/40 hover:-translate-y-1 hover:shadow-lg hover:shadow-accent/5"
            >
              <h3 className="text-lg font-semibold text-text-primary mb-2">
                {item.title}
              </h3>
              <p className="text-sm text-text-secondary leading-relaxed">
                {item.description}
              </p>
            </div>
          ))}
        </div>
      </SectionWrapper>

      {/* Sign-in panel */}
      <SectionWrapper>
        <div className="max-w-xl mx-auto text-center bg-surface-raised border border-border-subtle rounded-2xl p-8">
          <h2 className="text-2xl font-semibold text-text-primary mb-3">
            {signInPanel.title}
          </h2>
          <p className="text-text-secondary mb-6 leading-relaxed">
            {signInPanel.description}
          </p>
          <Button variant="primary" href={signInPanel.primaryCta.href}>
            {signInPanel.primaryCta.label}
          </Button>
        </div>
      </SectionWrapper>

      {/* Final CTA */}
      <SectionWrapper>
        <div className="max-w-2xl mx-auto text-center">
          <h2 className="text-2xl font-semibold text-text-primary mb-4">
            {resourcesCta.heading}
          </h2>
          <p className="text-text-secondary mb-6 leading-relaxed">
            {resourcesCta.body}
          </p>
          <Button variant="primary" href={resourcesCta.primaryCta.href}>
            {resourcesCta.primaryCta.label}
          </Button>
        </div>
      </SectionWrapper>
    </main>
  );
}
