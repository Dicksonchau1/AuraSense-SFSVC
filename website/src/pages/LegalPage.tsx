import { useParams } from 'react-router-dom';
import { PageContainer } from '../components/layout/PageContainer';
import { Tag } from '../components/ui/Tag';
import { legalPages } from '../content/legal';

export function LegalPage() {
  const { slug } = useParams<{ slug: string }>();
  const page = legalPages.find((p) => p.slug === slug);

  if (!page) {
    return (
      <section className="py-24">
        <PageContainer>
          <h1 className="text-2xl font-bold text-text-primary">Page not found</h1>
          <p className="mt-4 text-text-secondary">The requested legal page does not exist.</p>
        </PageContainer>
      </section>
    );
  }

  return (
    <section className="py-24">
      <PageContainer>
        <div className="max-w-3xl">
          <div className="flex items-center gap-3 mb-4">
            <Tag label="Legal" variant="default" />
            <Tag label={page.jurisdiction} variant="accent" />
          </div>
          <h1 className="text-3xl md:text-4xl font-bold text-text-primary mb-4">{page.title}</h1>
          <p className="text-sm text-text-muted mb-8">Last updated: {page.lastUpdated}</p>
          
          {/* TODO: Migrate full legal content from existing HTML pages */}
          <div className="bg-bg-surface border border-border-default rounded-sm p-8">
            <p className="text-sm text-text-secondary leading-relaxed">
              Full legal content for {page.title} will be migrated from the existing site. The authoritative versions are maintained at www.aurasensehk.com.
            </p>
            <p className="text-sm text-text-muted mt-4">
              For immediate enquiries, contact: support@aurasensehk.com
            </p>
          </div>
        </div>
      </PageContainer>
    </section>
  );
}
